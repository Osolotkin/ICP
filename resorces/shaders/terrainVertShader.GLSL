#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

out vec2 vsTexCoord;
out float vsFogRatio;

void main() {
    gl_Position = vec4(pos, 1.0);
    vsTexCoord = texCoord;
}