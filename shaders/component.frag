#version 450 core

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    vec4 color;
} fs_in;

out vec4 FragColor;

uniform vec3 uLightDir = normalize(vec3(0.5, 1.0, 0.3));
uniform vec3 uLightColor = vec3(1.0);
uniform vec3 uViewPos;
uniform float uAmbientStrength = 0.3;
uniform float uSpecularStrength = 0.5;
uniform float uShininess = 32.0;
uniform float uAlpha = 1.0;
uniform bool uUseVertexColor = true;
uniform vec4 uBaseColor = vec4(0.7, 0.7, 0.7, 1.0);

void main() {
    vec4 objectColor = uUseVertexColor ? fs_in.color : uBaseColor;

    // Ambient
    vec3 ambient = uAmbientStrength * uLightColor;

    // Diffuse
    vec3 norm = normalize(fs_in.normal);
    float diff = max(dot(norm, uLightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular
    vec3 viewDir = normalize(uViewPos - fs_in.fragPos);
    vec3 reflectDir = reflect(-uLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = uSpecularStrength * spec * uLightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor.rgb;
    FragColor = vec4(result, objectColor.a * uAlpha);
}
