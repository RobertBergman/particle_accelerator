#version 450 core

layout(location = 0) in vec4 aPositionSize;   // xyz = position, w = size
layout(location = 1) in vec4 aColorEnergy;    // rgb = color, a = energy

out VS_OUT {
    vec3 color;
    float size;
} vs_out;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 viewPos = uView * vec4(aPositionSize.xyz, 1.0);
    gl_Position = uProjection * viewPos;

    // Size attenuation based on distance
    float dist = length(viewPos.xyz);
    vs_out.size = aPositionSize.w * (500.0 / max(dist, 1.0));
    vs_out.color = aColorEnergy.rgb;
    gl_PointSize = vs_out.size;
}
