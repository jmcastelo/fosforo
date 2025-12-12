#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float size;
uniform float width;
uniform float height;
uniform float opacity;

void main()
{
    vec2 uv = texCoords.xy;
    vec2 tc = texCoords;
    if (size > 0.0) {
        float dx = size / width;
        float dy = size / height;
        tc = vec2(dx * (floor(uv.x / dx) + 0.5), dy * (floor(uv.y / dy) + 0.5));
    }
    vec3 srcColor = texture(inTexture, uv).rgb;
    vec3 dstColor = texture(inTexture, tc).rgb;
    fragColor = vec4(mix(srcColor, dstColor, opacity), 1.0);
}
