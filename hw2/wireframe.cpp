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

    //std::fstream file("draw2doutput.ppm", std::fstream::out);
    //pic.display(std::cout, 255);
    //file.close();

    return 0;
}

