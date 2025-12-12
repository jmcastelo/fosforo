#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float spatialKernel[49];
uniform vec2 offset[49];
uniform float rangeSigma;
uniform int numElements;
uniform float opacity;

void main()
{
    vec3 srcColor = texture(inTexture, texCoords).rgb;

    vec3 dstColor = vec3(0.0);
    float sum = 0.0;

    for (int i = 0; i < numElements; i++)
    {
        vec3 offsetColor = texture(inTexture, texCoords + offset[i]).rgb;

        float rangeKernel = exp(-0.5 * dot(srcColor - offsetColor, srcColor - offsetColor) / (rangeSigma * rangeSigma));
        float kernel = spatialKernel[i] * rangeKernel;

        dstColor += kernel * offsetColor;

        sum += kernel;
    }

    dstColor /= sum;

    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
