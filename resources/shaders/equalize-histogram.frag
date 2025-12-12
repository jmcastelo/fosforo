#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform int levels;
uniform vec2 offset[25];
uniform float opacity;

const int maxLevels = 256;

/*void main() {
    int numLevels = min(levels, maxLevels);

    vec3 histogram[maxLevels];

    for (int i = 0; i < numLevels; i++)
        histogram[i] = vec3(0.0);

    for (int i = 0; i < 25; i++)
    {
        vec3 color = texture(inTexture, texCoords + offset[i]).rgb;

        vec3 index = floor(color * (numLevels - 1.0));

        histogram[int(index.r)].r += 1.0;
        histogram[int(index.g)].g += 1.0;
        histogram[int(index.b)].b += 1.0;
    }

    vec3 cdf[maxLevels];
    cdf[0] = histogram[0] / 25.0;

    for (int i = 1; i < numLevels; i++)
        cdf[i] = histogram[i] / 25.0 + cdf[i - 1];

    vec3 srcColor = texture(inTexture, texCoords).rgb;
    vec3 srcIndex = floor(srcColor * (numLevels - 1.0));

    vec3 dstColor = vec3(cdf[int(srcIndex.r)].r, cdf[int(srcIndex.g)].g, cdf[int(srcIndex.b)].b);

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}*/

void main() {
    int numLevels = min(levels, maxLevels);

    int histogram[maxLevels];

    for (int i = 0; i < numLevels; i++)
        histogram[i] = 0;

    for (int i = 0; i < 25; i++)
    {
        vec3 color = texture(inTexture, texCoords + offset[i]).rgb;
        float brightness = (color.r + color.g + color.b) / 3.0;

        int index = int(brightness * (numLevels - 1));
        histogram[index]++;
    }

    float cdf[maxLevels];
    cdf[0] = histogram[0] / 25.0;

    for (int i = 1; i < numLevels; i++)
        cdf[i] = histogram[i] / 25.0 + cdf[i - 1];

    vec3 srcColor = texture(inTexture, texCoords).rgb;
    float srcBrightness = (srcColor.r + srcColor.g + srcColor.b) / 3.0;

    int srcIndex = int(srcBrightness * (numLevels - 1));

    vec3 dstColor = srcColor * cdf[srcIndex];

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}

