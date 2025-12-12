#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec3 gamma;
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 dstColor = pow(srcColor, gamma);
    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
