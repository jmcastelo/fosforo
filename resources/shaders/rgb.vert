#version 330 core

layout (location = 0) in vec2 texCoord;

out vec3 texColor;

uniform sampler2D texSampler;
uniform mat4 transform;

void main()
{
    vec4 color = texture(texSampler, texCoord);
    gl_Position = transform * vec4(color.rgb, 1.0);
    texColor = color.rgb;
}
