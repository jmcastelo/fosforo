#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform int redLevels;
uniform int greenLevels;
uniform int blueLevels;
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 levels = vec3(redLevels, greenLevels, blueLevels);
    vec3 dstColor = floor(levels * srcColor - 0.5) / (levels - 1.0);
    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
