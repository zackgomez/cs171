#include <fstream>
#include <iostream>
#include <cstdlib>
#include "wireframe.h"
#include "canvas.h"
#include "matrix.h"
#include "transforms.h"

void parse_file(std::istream &input, Scene *output);

void print_scene_info(const Scene &scene);
void render_scene(const Scene &scene, Canvas &canv);
Matrix4 worldToNDCMatrix(const Scene &scene);
void rasterizeEdge(const Vector3 &, const Vector3 &, const Matrix4 &, Canvas &);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "usage: wireframe xRes yRes\n";
        exit(1);
    }
    unsigned xRes, yRes;
    xRes = atof(argv[1]);
    yRes = atof(argv[2]);

    Scene scene;
    parse_file(std::cin, &scene);

    // Canvas dimensions are NDC
    Canvas canv(-1, 1, -1, 1, xRes, yRes);

    //print_scene_info(scene);
    render_scene(scene, canv);

    //std::fstream file("wireframe.ppm", std::fstream::out);
    canv.display(std::cout, 255);
    //file.close();

    return 0;
}

void render_scene(const Scene &scene, Canvas &canv)
{
    Matrix4 viewProjectionMatrix = worldToNDCMatrix(scene);

    std::vector<Separator>::const_iterator it = scene.separators.begin();
    for (; it != scene.separators.end(); it++)
    {
        //std::cout << "Model to world space matrix:\n" << it->transform;
        Matrix4 modelViewProjectionMatrix = viewProjectionMatrix * it->transform;
        const std::vector<Vector3>& points = it->points;
        const std::vector<int>& indices = it->indices;

        //std::cout << "Full transform matrix:\n" << modelViewProjectionMatrix;

        int firstInd = -1;
        int prevInd = -1;
        for (unsigned i = 0; i < indices.size(); i++)
        {
            int ind = indices[i];
            // ending index -> draw from prev to first and clear
            if (ind == -1)
            {
                // draw a line between points[prevInd] and points[firstInd]
                rasterizeEdge(points[prevInd], points[firstInd], modelViewProjectionMatrix, canv);
                firstInd = prevInd = -1;
                continue;
            }
            // First index, don't draw a line, just record points
            if (firstInd == -1)
            {
                firstInd = ind;
                prevInd = ind;
            }
            // Middle indices, draw a line between prev and current, update prev
            else
            {
                // draw a line between points[prevInd] and points[ind]
                rasterizeEdge(points[prevInd], points[ind], modelViewProjectionMatrix, canv);

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

    std::cout << "Separators...\n";
    for (std::vector<Separator>::const_iterator it = scene.separators.begin(); it != scene.separators.end(); it++)
    {
        const Separator& sep = *it;
        std::cout << " --- Separator Info ---\n";
        std::cout << "Transform:\n" << sep.transform;
        std::cout << "Points:\n";
        for (std::vector<Vector3>::const_iterator itt = sep.points.begin(); itt != sep.points.end(); itt++)
            std::cout << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cout << "Indexes:\n";
        for (std::vector<int>::const_iterator itt = sep.indices.begin(); itt != sep.indices.end(); itt++)
            if (*itt != -1)
                std::cout << *itt << ' ';
            else
                std::cout << '\n';
    }
}
