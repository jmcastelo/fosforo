#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float r;
uniform float opacity;

void main(void)
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 dstColor = r * srcColor * (1.0 - srcColor);
    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
