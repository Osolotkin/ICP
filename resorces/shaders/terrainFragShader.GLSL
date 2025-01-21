#version 410 core

uniform sampler2D ctex;

uniform vec4 ambientColor;
uniform vec4 fragFogColor;

// moon point light params
uniform vec3 moonPos;
uniform vec4 moonColor;
uniform vec4 moonDiffuseColor;

// headlights as point light params
uniform vec3 frontLightPos;
uniform vec3 frontLightDir;
uniform vec4 frontLightColor;
uniform vec4 frontLightDiffuseColor;

in float teFragHeight;
in vec2 teTexCoord;
in float teFogRatio;
in vec3 teFragPos;
in vec3 teNormal;

out vec4 FragColor;

vec4 calcDirectionalLight(vec4 lightColor, vec4 lightDiffuse, vec3 lightPos, vec3 lightDir, vec4 fragColor) {
    
    const float constant  = 1.0;
    const float linear    = 0.00001;
    const float quadratic = 0.000001;

    const float maxDotDir = 0.7;

    vec3 light2fragDir = normalize(lightPos - teFragPos);
    float dist = length(lightPos - teFragPos);

    float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));    
    attenuation = max(attenuation, 0.1);

    vec4 diffuse = lightDiffuse * max(dot(teNormal, light2fragDir), 0.0) * fragColor * attenuation;
    vec4 ambient = lightColor * fragColor * attenuation;

    float dotDir = clamp(dot(light2fragDir, -lightDir), maxDotDir, 1.0);
    float volume = (dotDir - maxDotDir) / (1.0 - maxDotDir);

    return (ambient + diffuse) * volume;

}

vec4 calcPointLight(vec4 lightColor, vec4 lightDiffuse, vec3 lightPos, vec4 fragColor) {
    
    const float constant  = 1.0;
    const float linear    = 0.0001;
    const float quadratic = 0.00001;

    vec3 lightDir = normalize(lightPos - teFragPos);
    float dist = length(lightPos - teFragPos);
    
    float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));    
    attenuation = max(attenuation, 0.1);
    
    vec4 ambient = lightColor * fragColor * attenuation;
    vec4 diffuse = lightDiffuse * max(dot(teNormal, lightDir), 0.0) * fragColor * attenuation;

    return ambient + diffuse;

}

void main() {

    float h = teFragHeight/256.0f;

    vec4 color = texture(ctex, teTexCoord);// * ambientColor;
    FragColor = calcPointLight(moonColor, moonDiffuseColor, moonPos, color);
    FragColor += calcDirectionalLight(frontLightColor, frontLightDiffuseColor, frontLightPos, frontLightDir, color);
    FragColor = mix(FragColor, fragFogColor, teFogRatio);//teFogRatio * color + (1.0f - teFogRatio) * fragFogColor;

}