#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 3) in vec4 aColor;

out vec4 vColor;

uniform mat4 uViewProjection;

void main() {
    vColor = aColor;
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
}
