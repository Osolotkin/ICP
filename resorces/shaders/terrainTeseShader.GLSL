#version 410 core

layout(quads, fractional_odd_spacing, ccw) in;

uniform sampler2D etex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 plPos;

in vec2 tcTexCoord[];
patch in vec3 tcPlPos;

out float teFragHeight;
out vec2 teTexCoord;
out float teFogRatio;
out vec3 teFragPos;
out vec3 teNormal;

float calcFogRatio(vec3 pos) {

    const float start = 5;
    const float end = 100;

    float dist = clamp(length(pos - plPos), start, end) - 5;
    return clamp(dist / (end - start), 0.0, 1.0);
    
}

void main() {

    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = tcTexCoord[0];
    vec2 t01 = tcTexCoord[1];
    vec2 t10 = tcTexCoord[2];
    vec2 t11 = tcTexCoord[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    teTexCoord = texCoord;
    teFragHeight = texture(etex, texCoord).r * 128;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0 + normal * teFragHeight;

    gl_Position = projection * view * model * p;
    teFogRatio = calcFogRatio(p.xyz);
    teFragPos = p.xyz;
    teNormal = normal.xyz;

}
