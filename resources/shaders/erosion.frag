#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 offset[8];
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 dstColor = texture(inTexture, texCoords + offset[0]).rgb;

    for (int i = 1; i < 8; i++)
        dstColor = min(dstColor,  texture(inTexture, texCoords + offset[i]).rgb);

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
