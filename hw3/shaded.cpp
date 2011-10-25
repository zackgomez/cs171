#include <fstream>
#include <iostream>
#include <cstdlib>
#include "shaded.h"
#include "canvas.h"
#include "matrix.h"
#include "transforms.h"
#include "raster.h"
void parse_file(std::istream &input, Scene *output);

void print_scene_info(const Scene &scene);
void render_scene(const Scene &scene, Canvas &canv, int shadingMode);
Matrix4 getCameraTransform(const Scene &scene);
Matrix4 worldToNDCMatrix(const Scene &scene);
Matrix4 createModelMatrix(const Transform &);
Matrix4 createNormalMatrix(const Transform &);
Vector3 lightFunc(const Vector3 &pos, const Vector3 &normal, const Material &material,
        const std::vector<Light>& lights, const Vector3 &camerapos);

static const int FLAT = 0;
static const int GOURAUD = 1;
static const int PHONG = 2;

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "usage: shaded n xRes yRes\n";
        exit(1);
    }
    unsigned lightMode, xRes, yRes;
    lightMode = atoi(argv[1]);
    xRes = atoi(argv[2]);
    yRes = atoi(argv[3]);

    if (lightMode != FLAT && lightMode != GOURAUD && lightMode != PHONG)
    {
        std::cerr << "Lighting mode must be 0, 1, or 2\n";
        exit(1);
    }

    Scene scene;
    parse_file(std::cin, &scene);

    // Canvas dimensions are NDC
    Canvas canv(-1, 1, -1, 1, xRes, yRes);

    print_scene_info(scene);
    render_scene(scene, canv, lightMode);

    //std::fstream file("shaded.ppm", std::fstream::out);
    canv.display(std::cout, 255);
    //file.close();

    return 0;
}

/**
 * This fragment processor does no extra processing.
 * It expects the data to be like positions data[0-2] and color data[3-5].
 */
struct simple_shader
{
    simple_shader(Canvas &canv) : canvas(canv) {}

    void operator()(int x, int y, float *data)
    {
        /*
        std::cerr << "Drawing pixel at (" << x << ' ' << y << ") - "
            << '[' << data[0] << ' ' << data[1] << ' ' << data[2] << "] "
            << "color [" << data[3] << ' ' << data[4] << ' ' << data[5] << "]\n";
            */

        //canvas.drawPixel(x, y, data[2], 1, 1, 1);
        canvas.drawPixel(x, y, data[2], data[3], data[4], data[5]);
    }

private:
    Canvas &canvas;
};

/**
 * This FragmentProcessor implements phong shading.  It expects the data to
 * include (x, y, z) ndc positions (0-2) and world position (3-5) and finally
 * the normal vector (6-8).
 */
struct phong_shader
{
    phong_shader(Canvas &canv, const Material &material,
            const std::vector<Light>& lights, const Vector3 &cameraPos) :
        canvas_(canv),
        material_(material),
        lights_(lights),
        cameraPos_(cameraPos)
    {}

    void operator()(int x, int y, float *data)
    {
        Vector3 worldCoord = makeVector3(data[3], data[4], data[5]);
        Vector3 normal = makeVector3(data[6], data[7], data[8]);
        Vector3 color = lightFunc(worldCoord, normal, material_, lights_, cameraPos_);

        canvas_.drawPixel(x, y, data[2], color(0), color(1), color(2));
    }

private:
    Canvas &canvas_;
    const Material &material_;
    const std::vector<Light> &lights_;
    const Vector3 &cameraPos_;
};

void render_scene(const Scene &scene, Canvas &canv, int shadingMode)
{
    initRaster(&canv);
    const Matrix4 viewProjectionMatrix = worldToNDCMatrix(scene);
    const Vector3 cameraPos = scene.camera.position;
    std::cerr << "CameraPos:\n" << cameraPos;

    std::vector<Separator>::const_iterator it = scene.separators.begin();
    for (; it != scene.separators.end(); it++)
    {
        std::cerr << " --- SEPARATOR ---\n";
        Matrix4 modelMatrix = make_identity<float,4>();
        Matrix4 normalMatrix = make_identity<float,4>();
        for (unsigned i = 0; i < it->transforms.size(); i++)
        {
            modelMatrix = modelMatrix * createModelMatrix(it->transforms[i]);
            normalMatrix = createNormalMatrix(it->transforms[i]) * normalMatrix;
        }
        const Matrix4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;

        std::cerr << "Model to world space matrix:\n" << modelMatrix;
        std::cerr << "World to NDC matrix:\n" << viewProjectionMatrix;
        std::cerr << "Full transform matrix:\n" << modelViewProjectionMatrix;
        std::cerr << "Normal matrix:\n" << normalMatrix;


        const std::vector<Vector3>& points = it->points;
        const std::vector<int>& indices = it->indices;
        const std::vector<Vector3>& normals = it->normals;
        const std::vector<int>& normindices = it->normalindices;

        int firstInd = -1;
        int prevInd = -1;
        int firstNormi = -1;
        int prevNormi = -1;
        for (unsigned i = 0; i < indices.size(); i++)
        {
            int ind = indices[i];
            int normi = normindices[i];
            // ending index -> reset
            if (ind == -1)
            {
                firstInd = prevInd = -1;
                firstNormi = prevNormi = -1;
            }
            // first index -> just record index
            else if (firstInd == -1)
            {
                firstInd = ind;
                firstNormi = normi;
            }
            // first index recorded, but not second -> just record second
            else if (prevInd == -1)
            {
                prevInd = ind;
                prevNormi = normi;
            }
            // both first and second indices recorded -> rasterize triange
            else
            {
                // Cache points
                Vector3 worldCoords[3] = {points[firstInd], points[prevInd], points[ind]};
                Vector3 ndcCoords[3] = {points[firstInd], points[prevInd], points[ind]};
                Vector3 norms[3] = {normals[firstNormi], normals[prevNormi], normals[normi]};
                // And update indexes
                prevInd = ind;
                prevNormi = normi;
                // Transform the points, do this now so we can check for backface culling
                for (int i = 0; i < 3; i++)
                {
                    Vector4 coord = modelViewProjectionMatrix * homogenize(ndcCoords[i]);
                    coord /= coord(3);
                    ndcCoords[i] = makeVector3(coord(0), coord(1), coord(2));

                    coord = modelMatrix * homogenize(worldCoords[i]);
                    coord /= coord(3);
                    worldCoords[i] = makeVector3(coord(0), coord(1), coord(2));
                }
                // Check for backface culling
                // Z coordinate of cross product (v2 - v1) X (v0 - v1)
                float z = (ndcCoords[2](0) - ndcCoords[1](0)) * (ndcCoords[0](1) - ndcCoords[1](1)) -
                          (ndcCoords[0](0) - ndcCoords[1](0)) * (ndcCoords[2](1) - ndcCoords[1](1));
                // If the triangle faces away.. don't draw
                if (z <= 0)
                    continue;

                /*
                std::cerr << "Drawing Triangle"
                    << ": [" << ndcCoords[0](0) << ' ' << ndcCoords[0](1) << ' ' << ndcCoords[0](2)
                    << "] - [" << ndcCoords[1](0) << ' ' << ndcCoords[1](1) << ' ' << ndcCoords[1](2)
                    << "] - [" << ndcCoords[2](0) << ' ' << ndcCoords[2](1) << ' ' << ndcCoords[2](2) << "]\n";
                    */


                // Now transform the normals
                for (int i = 0; i < 3; i++)
                {
                    Vector4 norm = normalMatrix * homogenize(norms[i]);
                    norm /= norm(3);
                    // Use a shortcut to normalize
                    norm(3) = 0; norm.normalize();
                    norms[i] = makeVector3(norm(0), norm(1), norm(2));
                }

                Vector3 color;
                // Calculate lighting once (FLAT)
                if (shadingMode == FLAT)
                    color = lightFunc((worldCoords[0] + worldCoords[1] + worldCoords[2])/3.0f,
                            (norms[0] + norms[1] + norms[2]) / 3.0f, it->material, scene.lights, cameraPos);

                vertex verts[3];
                // Create vertices
                for (int i = 0; i < 3; i++)
                {
                    Vector3 worldCoord = worldCoords[i];
                    Vector3 ndcCoord = ndcCoords[i];
                    Vector3 normal = norms[i];

                    if (i == 0)
                        std::cerr << "Normal Vector: [" << normal(0) << ' ' << normal(1) << ' ' << normal(2) << "]\n";

                    // Calculate the lighting (GOURUAD)
                    if (shadingMode == GOURAUD)
                        color = lightFunc(worldCoord, normal, it->material, scene.lights, cameraPos);

                    // Then stuff the vertex structure
                    // The data differs for flat/gouraud vs phong  see the
                    // definitions of simple_shader and phong_shader
                    const int NUM_DATA = 9;
                    verts[i].num_data = NUM_DATA;
                    verts[i].data = new float[NUM_DATA];
                    verts[i].data[0] = ndcCoord(0);
                    verts[i].data[1] = ndcCoord(1);
                    verts[i].data[2] = ndcCoord(2);
                    verts[i].data[3] = shadingMode == PHONG ? worldCoord(0) : color(0);
                    verts[i].data[4] = shadingMode == PHONG ? worldCoord(1) : color(1);
                    verts[i].data[5] = shadingMode == PHONG ? worldCoord(2) : color(2);
                    verts[i].data[6] = normal(0);
                    verts[i].data[7] = normal(1);
                    verts[i].data[8] = normal(2);
                }

                // RASTERIZE GO
                if (shadingMode == PHONG)
                    rasterizeTriangle(verts, phong_shader(canv, it->material, scene.lights, cameraPos));
                else
                    rasterizeTriangle(verts, simple_shader(canv));

                // clean up
                for (int i = 0; i < 3; i++)
                    delete[] verts[i].data;
            }
        }
    }
}

Matrix4 getCameraTransform(const Scene &scene)
{
    const Vector3 &pos = scene.camera.position;
    const Vector4 &rot = scene.camera.orientation;

    // world to camera
    // C^-1 = R^-1 * T^-1
    Matrix4 ret = make_rotation(rot(0), rot(1), rot(2), -rot(3));
    ret = ret * make_translation(-pos(0), -pos(1), -pos(2));
    return ret;
}

Matrix4 worldToNDCMatrix(const Scene &scene)
{
    const Camera &cam = scene.camera;

    Matrix4 ret = getCameraTransform(scene);
    //std::cout << "World to camera matrix:\n" << ret;

    // camera to NDC
    ret = make_perspective(cam.left, cam.right, cam.bottom, cam.top, cam.nearDistance, cam.farDistance) * ret;
    //std::cout << "Perspective matrix:\n" << make_perspective(cam.left, cam.right, cam.bottom, cam.top, cam.nearDistance, cam.farDistance);

    // Done ret = P * C^-1
    return ret;
}

Matrix4 createModelMatrix(const Transform &transform)
{
    Matrix4 trans = make_translation(transform.translation(0), transform.translation(1), transform.translation(2));
    Matrix4 scale = make_scaling(transform.scaling(0), transform.scaling(1), transform.scaling(2));
    Matrix4 rot = make_rotation(transform.rotation(0), transform.rotation(1), transform.rotation(2), transform.rotation(3));

    return trans * scale * rot;
}

Matrix4 createNormalMatrix(const Transform &transform)
{
    // Inverse scaling is 1/params
    Matrix4 scale = make_scaling(1/transform.scaling(0), 1/transform.scaling(1), 1/transform.scaling(2));
    // Inverse rotation is rotation around same vector, negative angle
    Matrix4 rot = make_rotation(transform.rotation(0), transform.rotation(1), transform.rotation(2), -transform.rotation(3));

    return (scale * rot).transpose();
}

void print_scene_info(const Scene &scene)
{
    std::cerr << "Printing out scene...\n";
    std::cerr << " --- Camera Info ---\n"
        << "position\n" << scene.camera.position
        << "orientation\n" << scene.camera.orientation
        << "nearDistance\n" << scene.camera.nearDistance << '\n'
        << "farDistance\n" << scene.camera.farDistance << '\n'
        << "left\n" << scene.camera.left << '\n'
        << "right\n" << scene.camera.right << '\n'
        << "top\n" << scene.camera.top << '\n'
        << "bottom\n" << scene.camera.bottom << '\n';

    std::cerr << "Lights...\n";
    for (std::vector<Light>::const_iterator it = scene.lights.begin(); it != scene.lights.end(); it++)
    {
        std::cerr << " --- Light Info ---\n"
            << "Position:\n" << it->position
            << "Color:\n" << it->color;
    }

    std::cerr << "Separators...\n";
    for (std::vector<Separator>::const_iterator it = scene.separators.begin(); it != scene.separators.end(); it++)
    {
        const Separator& sep = *it;
        std::cerr << " --- Separator Info ---\n";
        std::cerr << "Material:\n"
            << "ambientColor:\n" << sep.material.ambientColor
            << "diffuseColor:\n" << sep.material.diffuseColor
            << "specularColor:\n" << sep.material.specularColor
            << "shininess: " << sep.material.shininess << '\n';
        std::cerr << "Transforms:\n";
        for (std::vector<Transform>::const_iterator itt = sep.transforms.begin(); itt != sep.transforms.end(); itt++)
            std::cerr << "Translation:\n" << itt->translation
                << "Rotation:\n" << itt->rotation
                << "Scaling:\n" << itt->scaling;

        std::cerr << "Points:\n";
        for (std::vector<Vector3>::const_iterator itt = sep.points.begin(); itt != sep.points.end(); itt++)
            std::cerr << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cerr << "Indexes:\n";
        for (std::vector<int>::const_iterator itt = sep.indices.begin(); itt != sep.indices.end(); itt++)
            if (*itt != -1)
                std::cerr << *itt << ' ';
            else
                std::cerr << '\n';
        std::cerr << "Normals:\n";
        for (std::vector<Vector3>::const_iterator itt = sep.normals.begin(); itt != sep.normals.end(); itt++)
            std::cerr << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cerr << "Normal Indices:\n";
        for (std::vector<int>::const_iterator itt = sep.normalindices.begin(); itt != sep.normalindices.end(); itt++)
            if (*itt != -1)
                std::cerr << *itt << ' ';
            else
                std::cerr << '\n';
    }
}

Vector3 lightFunc(const Vector3 &pos, const Vector3 &normal, const Material &material,
        const std::vector<Light>& lights, const Vector3 &camerapos)
{
    Vector3 diffuse, specular;

    for (unsigned i = 0; i < lights.size(); i++)
    {
        // Calculate the diffuse contribution
        float NdotL = (normal.dot((lights[i].position - pos).normalize()));
        Vector3 ddiffuse = lights[i].color * NdotL;
        ddiffuse.clamp(0, HUGE_VAL);
        diffuse += ddiffuse;

        // Calculate the specular contribution
        float k = normal.dot(((camerapos - pos).normalize() + (lights[i].position - pos).normalize()).normalize());
        k = k > 0 ? k : 0; // zeroclip
        Vector3 dspecular = lights[i].color * powf(k, material.shininess);
        dspecular.clamp(0, HUGE_VAL);
        specular += dspecular;
    }

    diffuse.clamp(0, 1);
    Vector3 color = material.ambientColor + (diffuse ^ material.diffuseColor) + (specular ^ material.specularColor);
    color.clamp(0, 1);

    return color;
}
