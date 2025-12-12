#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

void main()
{
    fragColor = texture(inTexture, texCoords);
}
