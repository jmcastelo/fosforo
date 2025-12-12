#version 330 core

out vec4 fragColor;

in vec2 texCoords;

uniform sampler2D inTexture;

uniform float scaleX;
uniform float scaleY;

uniform float innerRadius;
uniform float outerRadius;

void main()
{
    vec2 xy = vec2(texCoords.x * scaleX, texCoords.y * scaleY);

    float r = distance(xy, vec2(0.5 * scaleX, 0.5 * scaleY));

    if (r < innerRadius || r > outerRadius)
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    else
        fragColor = texture(inTexture, texCoords);
}
