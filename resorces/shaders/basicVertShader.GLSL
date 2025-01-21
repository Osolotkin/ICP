#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 plPos;
uniform vec3 worldPos;

out vec2 vsTexCoord;
out float vsFogRatio;
out vec3 vsFragPos;
out vec3 vsNormal;

void main() {
    gl_Position = projection * view * (model * vec4(pos, 1.0) + vec4(worldPos, 0.0));// + vec4(worldPos, 1.0));
    vsTexCoord = texCoord;
    vsFragPos = worldPos + pos;
    vsNormal = normal;
    vsFogRatio = 1.0f / (1.0f + 0.5f * length(worldPos + pos - plPos));
}