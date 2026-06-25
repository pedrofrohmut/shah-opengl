#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "macros.h"

int main(void)
{
    // Creates a vertex (i.e. point) x, y, z, w
    // w=1 means it is a point. w=0 means it is a vector
    glm::vec4 vertex(1.0f, 5.0f, 1.0f, 1.0f);

    // Creates a model matrix for out geometry
    // Initialize with 1.0f for an identity matrix
    glm::mat4 model(1.0f);

    // Perform some transformation (i.e. moving us in the world). We are now in the world space
    // Our position is the vertex point. It is transformed by multipling the point be the matrix model

    // Matrix Scale - Double the vertex coords
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

    // Matrix Rotation
    glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 1, 0));

    // Matrix Translation - Point will move -2 in the z axis
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));

    // Operations are applied right to left.
    // The order that the operations are applied matter.

    // Translate first, then rotate and then scale.
    //model = s * r * t; // -2, 10, 2, 1

    // Scale first, then rotate and then translate.
    model = t * r * s; // -2, 10, -4, 1

    // Print matrix - trick to print matrix for debugging
    println_(glm::to_string(model[0]));
    println_(glm::to_string(model[1]));
    println_(glm::to_string(model[2]));
    println_(glm::to_string(model[3]));

    // Now apply our 'model' matrix to the vertex
    glm::vec4 worldspace_vertex = (model * vertex);

    println_('\n' << "our vertex in world space");
    println_(glm::to_string(worldspace_vertex));

    return 0;
}
