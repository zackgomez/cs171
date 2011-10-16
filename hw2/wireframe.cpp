#include <fstream>
#include <iostream>
#include <cstdlib>
#include "wireframe.h"

void parse_file(std::istream &input, Scene *output);

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
    for (std::vector<Separator>::iterator it = scene.separators.begin(); it != scene.separators.end(); it++)
    {
        Separator& sep = *it;
        std::cout << " --- Separator Info ---\n";
        std::cout << "Transform:\n" << sep.transform;
        std::cout << "Points:\n";
        for (std::vector<Vector3>::iterator itt = sep.points.begin(); itt != sep.points.end(); itt++)
            std::cout << (*itt)(0) << ' ' << (*itt)(1) << ' ' << (*itt)(2) << '\n';
        std::cout << "Indexes:\n";
        for (std::vector<int>::iterator itt = sep.indices.begin(); itt != sep.indices.end(); itt++)
            if (*itt != -1)
                std::cout << *itt << ' ';
            else
                std::cout << '\n';
    }

    //std::fstream file("draw2doutput.ppm", std::fstream::out);
    //pic.display(std::cout, 255);
    //file.close();

    return 0;
}

