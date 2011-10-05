#include "canvas.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

void parse_file(std::istream &input, Canvas *output);

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        std::cerr << "usage: draw2d xmin xmax ymin ymax xRes yRes\n";
        exit(1);
    }
    float xmin, xmax, ymin, ymax;
    unsigned xRes, yRes;
    xmin = atof(argv[1]);
    xmax = atof(argv[2]);
    ymin = atof(argv[3]);
    ymax = atof(argv[4]);
    xRes = atoi(argv[5]);
    yRes = atoi(argv[6]);

    Canvas pic(xmin, xmax, ymin, ymax, xRes, yRes);
    parse_file(std::cin, &pic);

    std::fstream file("draw2doutput.ppm", std::fstream::out);
    pic.display(file, 255);
    file.close();

    return 0;
}

