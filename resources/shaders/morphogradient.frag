#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 dilationOffset[8];
uniform vec2 erosionOffset[8];
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;

    vec3 dilationColor = texture(inTexture, texCoords + dilationOffset[0]).rgb;

    for (int i = 1; i < 8; i++)
        dilationColor = max(dilationColor,  texture(inTexture, texCoords + dilationOffset[i]).rgb);

    vec3 erosionColor = texture(inTexture, texCoords + erosionOffset[0]).rgb;

    for (int i = 1; i < 8; i++)
        erosionColor = min(erosionColor,  texture(inTexture, texCoords + erosionOffset[i]).rgb);

    vec3 dstColor = dilationColor - erosionColor;
    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
