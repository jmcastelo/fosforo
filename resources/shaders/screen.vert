#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 tex;

uniform mat4 transform;

out vec2 texCoords;

void main()
{
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    texCoords = tex;
}
