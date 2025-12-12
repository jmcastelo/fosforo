#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform vec2 offset[9];
uniform float opacity;

void main()
{
    vec3 color[9];

    for (int i = 0; i < 9; i++)
        color[i] += texture(inTexture, texCoords + offset[i]).rgb;

    // Sort colors

    int i = 1;
    while (i < 9)
    {
        vec3 c = color[i];
        ivec3 j = ivec3(i - 1);
        while (j.x >= 0 && color[j.x].r > c.r)
        {
            color[j.x + 1].r = color[j.x].r;
            j.x--;
        }
        color[j.x + 1].r = c.r;
        while (j.y >= 0 && color[j.y].g > c.g)
        {
            color[j.y + 1].g = color[j.y].g;
            j.y--;
        }
        color[j.y + 1].g = c.g;
        while (j.z >= 0 && color[j.z].b > c.b)
        {
            color[j.z + 1].b = color[j.z].b;
            j.z--;
        }
        color[j.z + 1].b = c.b;
        i++;
    }

    vec3 srcColor = texture(inTexture, texCoords).rgb;
    fragColor = vec4(mix(srcColor, color[4], opacity), 1.0);
}
