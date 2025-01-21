#version 410 core

uniform sampler2D tex;
uniform vec4 ambientColor;
uniform vec4 fragFogColor;

// moon point light params
uniform vec3 moonPos;
uniform vec4 moonColor;
uniform vec4 moonDiffuseColor;

uniform vec3 frontLightPos;
uniform vec3 frontLightDir;
uniform vec4 frontLightColor;
uniform vec4 frontLightDiffuseColor;

in vec2 vsTexCoord;
in float vsFogRatio;
in vec3 vsFragPos;
in vec3 vsNormal;

out vec4 FragColor;

vec4 calcPointLight(vec4 lightColor, vec4 lightDiffuse, vec3 lightPos, vec4 fragColor) {
    
    const float constant  = 1.0;
    const float linear    = 0.0001;
    const float quadratic = 0.00001;

    const float shininess = 80;
    const vec4 lightSpecular = vec4(0.6, 0.6, 0.8, 1.0);

    vec3 lightDir = normalize(lightPos - vsFragPos);
    float dist = length(lightPos - vsFragPos);
    
    float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));    
    attenuation = max(attenuation, 0.1);
    
    vec4 ambient = lightColor * fragColor * attenuation;
    vec4 diffuse = lightDiffuse * max(dot(vsNormal, lightDir), 0.0) * fragColor * attenuation;
    vec4 specular = lightSpecular * pow(max(dot(reflect(-lightDir, normal), lightDir), 0.0), shininess) * fragColor * attenuation;

    return ambient + diffuse + specular;

}

void main() {
    
    vec4 color = texture(tex, vsTexCoord);
    color = calcPointLight(moonColor, moonDiffuseColor, moonPos, color);
    FragColor = color;
    //FragColor = vsFogRatio * color + (1.0f - vsFogRatio) * fragFogColor;

}