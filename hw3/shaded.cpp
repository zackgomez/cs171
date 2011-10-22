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
void render_scene(const Scene &scene, Canvas &canv);
Matrix4 worldToNDCMatrix(const Scene &scene);
Matrix4 createModelMatrix(const Transform &);
Matrix4 createNormalMatrix(const Transform &);
Vector3 lightFunc(const Vector3 &normal, const Vector3 &pos, const Material &material,
        const std::vector<Light>& lights, const Vector3 &camerapos);

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

    Scene scene;
    parse_file(std::cin, &scene);

    // Canvas dimensions are NDC
    Canvas canv(-1, 1, -1, 1, xRes, yRes);

    print_scene_info(scene);
    render_scene(scene, canv);

    //std::fstream file("shaded.ppm", std::fstream::out);
    canv.display(std::cout, 255);
    //file.close();

    return 0;
}

struct debug_pixel
{
    debug_pixel(Canvas &canv) : canvas(canv) {}

    void operator()(int x, int y, float *data)
    {
        /*
        std::cerr << "Drawing pixel at (" << x << ' ' << y << ") - "
            << '[' << data[0] << ' ' << data[1] << ' ' << data[2] << "] "
            << "normal [" << data[3] << ' ' << data[4] << ' ' << data[5] << "]\n";
            */

        //canvas.drawPixel(x, y, data[2], 1, 1, 1);
        canvas.drawPixel(x, y, data[2], data[3], data[4], data[5]);
    }

private:
    Canvas &canvas;
};

void render_scene(const Scene &scene, Canvas &canv)
{
    initRaster(&canv);
    Matrix4 viewProjectionMatrix = worldToNDCMatrix(scene);

    std::vector<Separator>::const_iterator it = scene.separators.begin();
    for (; it != scene.separators.end(); it++)
    {
        Matrix4 modelMatrix = make_identity<float,4>();
        Matrix4 normalMatrix = make_identity<float,4>();
        for (unsigned i = 0; i < it->transforms.size(); i++)
        {
            modelMatrix = modelMatrix * createModelMatrix(it->transforms[i]);
            normalMatrix = normalMatrix * createNormalMatrix(it->transforms[i]);
        }
        std::cerr << "Model to world space matrix:\n" << modelMatrix;
        //std::cout << "Normal matrix:\n" << normalMatrix;
        Matrix4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;
        //std::cout << "Full transform matrix:\n" << modelViewProjectionMatrix;


        const std::vector<Vector3>& points = it->points;
        const std::vector<int>& indices = it->indices;
        const std::vector<Vector3>& normals = it->normals;
        const std::vector<int>& normindices = it->normalindices;

        // TODO......
        const Vector3 cameraPos;

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
                Vector3 pts[3] = {points[firstInd], points[prevInd], points[ind]};
                Vector3 norms[3] = {normals[firstNormi], normals[prevNormi], normals[normi]};
                // And update indexes
                prevInd = ind;
                prevNormi = normi;
                // Transform the points, do this now so we can check for backface culling
                for (int i = 0; i < 3; i++)
                {
                    Vector4 coord = modelViewProjectionMatrix * homogenize(pts[i]);
                    coord /= coord(3);
                    pts[i](0) = coord(0); pts[i](1) = coord(1); pts[i](2) = coord(2);
                }
                std::cerr << "Rasterizing triangle <" << pts[0](0) << ' ' << pts[0](1) << ' '
                    << pts[0](2) << "> - <" << pts[1](0) << ' ' << pts[1](1) << ' '
                    << pts[1](2) << "> - <" << pts[2](0) << ' ' << pts[2](1) << ' '
                    << pts[2](2) << ">\n";
                // Check for backface culling
                // Z coordinate of cross product (v2 - v1) X (v0 - v1)
                float z = (pts[2](0) - pts[1](0)) * (pts[0](1) - pts[1](1)) -
                          (pts[0](0) - pts[1](0)) * (pts[2](1) - pts[1](1));
                // If the triangle faces away.. don't draw
                if (z <= 0)
                    continue;

                vertex verts[3];
                // Create vertices
                for (int i = 0; i < 3; i++)
                {
                    Vector3 coord = pts[i];
                    // Transform the normal vector
                    Vector4 norm = homogenize(norms[i]);
                    norm = normalMatrix * norm;
                    norm /= norm(3);
                    // Use a shortcut to normalize
                    norm(3) = 0; norm.normalize();
                    Vector3 normal; normal(0) = norm(0); normal(1) = norm(1); normal(2) = norm(2);

                    // Calculate the lighting
                    Vector3 color = lightFunc(coord, normal, it->material, scene.lights, cameraPos);
                    std::cerr << "Calculated color: (" << color(0) << ' ' << color(1) << ' ' << color(2) << ")\n";

                    // Then stuff the vertex structure
                    const int NUM_DATA = 6;
                    verts[i].num_data = NUM_DATA;
                    verts[i].data = new float[NUM_DATA];
                    verts[i].data[0] = coord(0);
                    verts[i].data[1] = coord(1);
                    verts[i].data[2] = coord(2);
                    verts[i].data[3] = color(0);
                    verts[i].data[4] = color(1);
                    verts[i].data[5] = color(2);
                }

                // RASTERIZE GO
                rasterizeTriangle(verts, debug_pixel(canv));

                // clean up
                for (int i = 0; i < 3; i++)
                    delete[] verts[i].data;
            }
        }
    }
}

Matrix4 worldToNDCMatrix(const Scene &scene)
{
    const Camera &cam = scene.camera;
    const Vector3 &pos = scene.camera.position;
    const Vector4 &rot = scene.camera.orientation;

    // world to camera
    // C^-1 = R^-1 * T^-1
    Matrix4 ret = make_rotation(rot(0), rot(1), rot(2), -rot(3));
    ret = ret * make_translation(-pos(0), -pos(1), -pos(2));
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

    /*
    for (unsigned i = 0; i < lights.size(); i++)
    {
    }
    */

    std::cerr << "diffuse:\n" << (diffuse ^ material.diffuseColor);

    Vector3 ret = material.ambientColor;
    /*
    std::cerr << "Color?\n" << ret;
    ret = ret + (diffuse ^ material.diffuseColor);
    std::cerr << "Color?\n" << ret;
    ret = ret + (specular ^ material.specularColor);
    std::cerr << "Color?\n" << ret;
    */

    return ret;
}
