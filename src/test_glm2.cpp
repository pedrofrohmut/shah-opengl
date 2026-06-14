#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "macros.h"

void main2d()
{
    println_("# Main 2d ############################################################ \n");

    // Create the vectors
    glm::vec3 A(3.0f, 4.0f, 0.0f);
    glm::vec3 B(0.0f, 7.0f, 0.0f);

    // Length == sqr(x_length^2 + y_length^2)
    printf_("Length A: {}\n", glm::length(A));
    // printf_("B: {}\n", glm::length(B));

    // Normalize the values to be between 0.0f and 1.0f
    printf_("Normalized A: {}\n", glm::to_string(glm::normalize(A)));

    // A-hat
    printf_("A-hat: {}\n", glm::length(glm::normalize(A)));

    // Dot is: (Ax * Bx) + (Ay * By) + (Az * Bz)
    printf_("Dot: {}\n", glm::dot(A, B));

    const float dotproduct = glm::dot(glm::normalize(A), glm::normalize(B));
    printf_("Dotproduct: {}\n", dotproduct);
    const float angle = std::acos(dotproduct) * 180.0f / M_PI;
    printf_("Angle: {}\n", angle);

    println_("\n");
}

void main3d()
{
    println_("# Main 3d ############################################################ \n");

    // Create the vectors
    glm::vec3 A(4.0f, 0.0f, 0.0f);
    glm::vec3 B(0.0f, 4.0f, 0.0f);

    // Cross Product gives you a perpendicular vector to other 2 vectors
    glm::vec3 C = glm::cross(A, B);

    printf_("Cross Product: {}\n", glm::to_string(C));

    println_("\n");
}

int main(void)
{
    main2d();
    main3d();
    return 0;
}
