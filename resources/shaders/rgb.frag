#version 330 core

in vec3 texColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(texColor, 1.0);
}
