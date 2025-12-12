#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float kernel[500];
uniform float centerElement;
uniform vec2 offset[500];
uniform int numElements;
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 dstColor = centerElement * srcColor;

    for (int i = 0; i < numElements; i++)
        dstColor += kernel[i] * texture(inTexture, texCoords + offset[i]).rgb;

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
