#version 330 core

in vec2 texCoords;
out vec4 fragColor;

uniform sampler2DArray inArrayTex;
uniform float weights[32];

void main() {
    int layerCount = textureSize(inArrayTex, 0).z;

    vec3 blend = vec3(0.0);

    for (int i = 0; i < layerCount; i++) {
        blend += texture(inArrayTex, vec3(texCoords, float(i))).rgb * weights[i];
    }

    fragColor = vec4(blend, 1.0);
}
