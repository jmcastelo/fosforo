#version 330 core

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2D inTexture;

void main()
{
    fragColor = texture(inTexture, texCoords);
}
