#version 410 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 vertexColors;

uniform float u_offsety; // uniform variable
uniform float u_offsetx; // uniform variable

out vec3 v_vertexColors;

void main()
{
    v_vertexColors = vertexColors;
    gl_Position = vec4(position.x + u_offsetx, position.y + u_offsety, position.z, 1.0f);
}
