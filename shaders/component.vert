#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aColor;

out VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 texCoords;
    vec4 color;
} vs_out;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vs_out.fragPos = worldPos.xyz;
    vs_out.normal = normalize(uNormalMatrix * aNormal);
    vs_out.texCoords = aTexCoords;
    vs_out.color = aColor;

    gl_Position = uProjection * uView * worldPos;
}
