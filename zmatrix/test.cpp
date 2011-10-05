#include "matrix.h"
#include "transforms.h"
#include <iostream>

int main(int argc, char **argv)
{
    Matrix3 mat;
    std::cout << "Empty matrix:\n";
    std::cout << mat;

    // Test assignment
    mat(0, 0) = 1;
    mat(1, 1) = 1;
    mat(2, 2) = 1;

    // Test compound scalar ops
    std::cout << "\n\nTesting scalar ops... ID * 5 / 2 + 3 - 1.5:\n";
    mat *= 5;
    mat /= 2;
    mat += 3;
    mat -= 1.5;
    std::cout << mat;

    // Test copy constructor
    Matrix3 mat2(mat);
    mat2(1, 2) = 5;

    // Test Transpose
    std::cout << "\n\nTransposition test\n" << mat2 << "   TRANSPOSE   \n" << mat2.transpose();

    Vector3 vec1;
    vec1(0) = 1;
    vec1(1) = 2;
    vec1(2) = 3;
    std::cout << "\n\nSimple vector:\n" << vec1;

    std::cout << "\n\nMultiplication test\n" << mat2 << " *** \n" << vec1 << " === \n";
    std::cout << mat2 * vec1;

    std::cout << "\n\nMultiplication test\n" << mat2 << " *** \n" << mat2.transpose() << " === \n";
    std::cout << mat2 * mat2.transpose();

    std::cout << "\n\nMagnitude test\n" << vec1 << " MAG \n" << vec1.magnitude();

    Vector3 normalized1(vec1);
    normalized1.normalize();
    std::cout << "\n\nNormalization test\n" << vec1 << " NORMALIZED \n" << normalized1;
    std::cout << " DOT OF ITSELF (should be 1) == " << normalized1.dot(normalized1) << '\n';

    Matrix4 ident = make_identity<float, 4>();
    std::cout << "\n\nIdentity matrix\n" << ident;

    Matrix3 mat3;
    mat3(0,0) = 5;
    mat3(0,1) = 4;
    mat3(0,2) = 3;
    mat3(1,0) = 8;
    mat3(1,1) = 1;
    mat3(1,2) = 3;
    mat3(2,0) = 9;
    mat3(2,1) = 5;
    mat3(2,2) = 7;

    Matrix3 mat3inv = mat3.inverse();

    std::cout << "\n\nInverse Matrix Test\n" << mat3 << " INVERSE \n" << mat3inv;
    std::cout << "A * Ainv =\n" << mat3 * mat3inv << '\n';

    std::cout << "\n\nTranslation matrix test\n" << "Translating (4,5,6)\n" << make_translation(4,5,6) << '\n';

    std::cout << "\n\nScaling matrix test\n" << "Scaling by <4 5 6>\n" << make_scaling(4,5,6) << '\n';

    std::cout << "\n\nRotation matrix test\n" << "Rotating by 45deg around <1 1 1>\n" << make_rotation(1,1,1, M_PI/4) << '\n';
    return 0;
}
