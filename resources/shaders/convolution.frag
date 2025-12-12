#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float kernel[9];
uniform vec2 offset[9];
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 dstColor = vec3(0.0);

    for (int i = 0; i < 9; i++)
        dstColor += kernel[i] * texture(inTexture, texCoords + offset[i]).rgb;

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
