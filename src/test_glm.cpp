#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/glm.hpp> // swizzle
#include <glm/gtx/string_cast.hpp> // to_string

#include "./macros.h"

int main(void)
{
    // Vec3 x, y, z coords
    glm::vec3 A(1.0f, 1.0f, 1.0f);
    glm::vec3 B(0.5f, 1.0f, 0.0f);

    // Dot
    float t = glm::dot(glm::normalize(A), glm::normalize(B));
    printf_("dot(A, B) is: {}\n", t);

    // Snizzle
    glm::vec3 Foo(0.3f, 0.3f, 0.3f);
    printf_("Foo: {}, A.xxx: {}\n", glm::to_string(Foo), glm::to_string(A.xxx()));
    glm::vec3 C = B.xxx();
    printf_("B: {}, C: {}\n", glm::to_string(B), glm::to_string(C));

    // Normalize
    println_("           Vec3 A: " << glm::to_string(A));
    println_("           Vec3 B: " << glm::to_string(B));
    println_("Normalized Vec3 A: " << glm::to_string(glm::normalize(A)));
    println_("Normalized Vec3 B: " << glm::to_string(glm::normalize(B)));

    // Matrices
    glm::mat4 mat(1.0f);
    printf_("Mat4: {}\n", glm::to_string(mat));
    println_("Mat4 mat: " << glm::to_string(mat));

    return 0;
}
