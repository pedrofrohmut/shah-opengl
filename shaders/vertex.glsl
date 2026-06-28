#version 410 core

layout(location=0) in vec3 positions;
layout(location=1) in vec3 vertexColors;

uniform mat4 u_ModelMatrix;

out vec3 v_vertexColors;

void main()
{
    v_vertexColors = vertexColors;

    vec4 newPositions = u_ModelMatrix * vec4(positions, 1.0f);

    gl_Position = vec4(newPositions.x, newPositions.y, newPositions.z, 1.0f);
}
