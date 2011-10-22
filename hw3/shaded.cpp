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
void rasterizeEdge(const Vector3 &, const Vector3 &, const Matrix4 &, Canvas &);
Matrix4 createModelMatrix(const Transform &);
Matrix4 createNormalMatrix(const Transform &);

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

    //print_scene_info(scene);
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
            << '[' << data[0] << ' ' << data[1] << ' ' << data[2] << "]\n";
            */

        canvas.drawPixel(x, y, data[2], 1, 1, 1);
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
        //std::cout << "Model to world space matrix:\n" << modelMatrix;
        //std::cout << "Normal matrix:\n" << normalMatrix;
        Matrix4 modelViewProjectionMatrix = viewProjectionMatrix * modelMatrix;
        //std::cout << "Full transform matrix:\n" << modelViewProjectionMatrix;


        const std::vector<Vector3>& points = it->points;
        const std::vector<int>& indices = it->indices;

        int firstInd = -1;
        int prevInd = -1;
        for (unsigned i = 0; i < indices.size(); i++)
        {
            int ind = indices[i];
            // ending index -> reset
            if (ind == -1)
                firstInd = prevInd = -1;
            // first index -> just record index
            else if (firstInd == -1)
                firstInd = ind;
            // first index recorded, but not second -> just record second
            else if (prevInd == -1)
                prevInd = ind;
            // both first and second indices recorded -> rasterize triange
            else
            {
                Vector3 pts[3] = {points[firstInd], points[prevInd], points[ind]};
                vertex verts[3];
                // Create vertices
                for (int i = 0; i < 3; i++)
                {
                    // First transform the point
                    Vector4 coord = modelViewProjectionMatrix * homogenize(pts[i]);
                    coord /= coord(3);

                    // Then stuff the vertex structure
                    verts[i].num_data = 3;
                    verts[i].data = new float[3];
                    verts[i].data[0] = coord(0);
                    verts[i].data[1] = coord(1);
                    verts[i].data[2] = coord(2);
                    /*
                    std::cout << "Sending vertex - [" << verts[i].data[0] << ' '
                        << verts[i].data[1] << ' ' << verts[i].data[2] << "]\n";
                        */
                }

                // RASTERIZE GO
                rasterizeTriangle(verts, debug_pixel(canv));

                // clean up
                for (int i = 0; i < 3; i++)
                    delete[] verts[i].data;

                // And update prevInd
                prevInd = ind;
            }
        }
    }
}

void rasterizeEdge(const Vector3& a, const Vector3& b, const Matrix4& fullTransform, Canvas &canv)
{
    // extend to homogenized coords
    Vector4 ah, bh;
    ah(0) = a(0); ah(1) = a(1); ah(2) = a(2); ah(3) = 1;
    bh(0) = b(0); bh(1) = b(1); bh(2) = b(2); bh(3) = 1;

    //std::cout << "Pre transform (" << ah(0) << ' ' << ah(1) << ' ' << ah(2) << ") to (" <<
        //bh(0) << ' ' << bh(1) << ' ' << bh(2) << ")\n";

    // Transform to NDC
    ah = fullTransform * ah;
    bh = fullTransform * bh;

    // re homogenize
    ah /= ah(3);
    bh /= bh(3);

    //std::cout << "Drawing from (" << ah(0) << ' ' << ah(1) << ' ' << ah(2) << ") to (" <<
        //bh(0) << ' ' << bh(1) << ' ' << bh(2) << ")\n";

    canv.drawLine(ah(0), ah(1), bh(0), bh(1));
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
    std::cout << "Printing out scene...\n";
    std::cout << " --- Camera Info ---\n"
        << "position\n" << scene.camera.position
        << "orientation\n" << scene.camera.orientation
        << "nearDistance\n" << scene.camera.nearDistance << '\n'
        << "farDistance\n" << scene.camera.farDistance << '\n'
        << "left\n" << scene.camera.left << '\n'
        << "right\n" << scene.camera.right << '\n'
        << "top\n" << scene.camera.top << '\n'
        << "bottom\n" << scene.camera.bottom << '\n';

    std::cout << "Lights...\n";
    for (std::vector<Light>::const_iterator it = scene.lights.begin(); it != scene.lights.end(); it++)
    {
        std::cout << " --- Light Info ---\n"
            << "Position:\n" << it->position
            << "Color:\n" << it->color;
    }

    std::cout << "Separators...\n";
    for (std::vector<Separator>::const_iterator it = scene.separators.begin(); it != scene.separators.end(); it++)
    {
        const Separator& sep = *it;
        std::cout << " --- Separator Info ---\n";
        std::cout << "Material:\n"
            << "ambientColor:\n" << sep.material.ambientColor
            << "diffuseColor:\n" << sep.material.diffuseColor
            << "specularColor:\n" << sep.material.specularColor
            << "shininess: " << sep.material.shininess << '\n';
        std::cout << "Transforms:\n";
        for (std::vector<Transform>::const_iterator itt = sep.transforms.begin(); itt != sep.transforms.end(); itt++)
            std::cout << "Translation:\n" << itt->translation
                << "Rotation:\n" << itt->rotation
                << "Scaling:\n" << itt->scaling;

        std::cout << "Points:\n";
        for (std::vector<Vector3>::const_iterator itt = sep.points.begin(); itt != sep.points.end(); itt++)
            std::cout << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cout << "Indexes:\n";
        for (std::vector<int>::const_iterator itt = sep.indices.begin(); itt != sep.indices.end(); itt++)
            if (*itt != -1)
                std::cout << *itt << ' ';
            else
                std::cout << '\n';
        std::cout << "Normals:\n";
        for (std::vector<Vector3>::const_iterator itt = sep.normals.begin(); itt != sep.normals.end(); itt++)
            std::cout << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cout << "Normal Indices:\n";
        for (std::vector<int>::const_iterator itt = sep.normalindices.begin(); itt != sep.normalindices.end(); itt++)
            if (*itt != -1)
                std::cout << *itt << ' ';
            else
                std::cout << '\n';
    }
}
