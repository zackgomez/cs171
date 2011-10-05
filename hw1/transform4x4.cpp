#include <iostream>
#include <fstream>
#include <cstdlib>
#include "matrix.h"

Matrix4 parse_file(std::istream &input);

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "usage: transform4x4 FILENAME\n";
        exit(1);
    }

    std::fstream file(argv[1]);
    Matrix4 transform = parse_file(file);

    std::cout << "Transform matrix:\n" << transform << '\n';

    return 0;
}
