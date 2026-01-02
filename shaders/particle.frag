#version 450 core

in VS_OUT {
    vec3 color;
    float size;
} fs_in;

out vec4 FragColor;

void main() {
    // Create circular particle with soft edges
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    if (dist > 0.5) {
        discard;
    }

    // Soft glow effect
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);

    // Add bloom-like brightness at center
    float brightness = 1.0 + (1.0 - dist * 2.0) * 0.5;

    FragColor = vec4(fs_in.color * brightness, alpha);
}
