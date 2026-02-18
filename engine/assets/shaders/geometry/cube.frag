#version 410 core
// Phase 3 — simple diffuse shading with checkerboard texture

in vec2 vUV;
in vec3 vNormal;
in vec3 vWorldPos;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    vec3 albedo  = texture(u_Texture, vUV).rgb;
    // Simple directional lighting from above-right
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    float diff    = max(dot(vNormal, lightDir), 0.0);
    vec3 ambient  = albedo * 0.25;
    FragColor     = vec4(ambient + albedo * diff * 0.75, 1.0);
}
