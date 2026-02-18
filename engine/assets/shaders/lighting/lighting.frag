#version 410 core
#include "../common/uniforms.glsl"
#include "../common/pbr.glsl"
#include "../common/shadow.glsl"

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_GNormal;    // world-space normal
uniform sampler2D u_GAlbedo;    // albedo
uniform sampler2D u_GMaterial;  // metallic, roughness, ao
uniform sampler2D u_GDepth;     // hardware depth
uniform sampler2D u_ShadowMap;  // directional shadow map

void main()
{
    float depth = texture(u_GDepth, vUV).r;

    // Sky / background
    if (depth >= 1.0) {
        FragColor = vec4(0.05, 0.07, 0.12, 1.0);
        return;
    }

    // Reconstruct world position from depth buffer.
    // Standard OpenGL: depth ∈ [0,1], NDC_z = depth*2-1 ∈ [-1,1]
    vec4 ndcPos  = vec4(vUV * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 worldP4 = u_InvViewProjection * ndcPos;
    vec3 worldPos = worldP4.xyz / worldP4.w;

    // Sample G-buffer
    vec3  N        = normalize(texture(u_GNormal,   vUV).rgb);
    vec3  albedo   = texture(u_GAlbedo,   vUV).rgb;
    vec4  mat4v    = texture(u_GMaterial, vUV);
    float metallic  = mat4v.r;
    float roughness = max(mat4v.g, 0.04);   // clamp to avoid singularity
    float ao        = mat4v.b;

    vec3 V = normalize(u_CameraPos - worldPos);

    // Directional light — L points from surface toward light
    vec3 L = normalize(-u_LightDir);

    // Cook-Torrance BRDF × light radiance
    vec3 radiance = u_LightColor * u_LightIntensity;
    vec3 Lo       = BRDF(N, V, L, albedo, metallic, roughness) * radiance;

    // PCF shadow
    vec4 shadowCoord = u_LightSpaceMatrix * vec4(worldPos, 1.0);
    shadowCoord.xyz /= shadowCoord.w;
    shadowCoord.xyz  = shadowCoord.xyz * 0.5 + 0.5;
    float shadow     = PCFShadow(u_ShadowMap, shadowCoord.xyz);
    Lo              *= shadow;

    // Ambient (hemisphere approximation × AO)
    vec3 ambient = vec3(0.05) * albedo * ao;

    FragColor = vec4(ambient + Lo, 1.0);
}
