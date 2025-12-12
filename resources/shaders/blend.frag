#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;
uniform sampler2D outTexture;

uniform float blendFactor;
uniform float outFactor;

void main()
{
    fragColor = vec4(outFactor * texture(outTexture, texCoords).rgb + blendFactor * texture(inTexture, texCoords).rgb, 1.0);
}
