#version 410 core

in vec3 vWorldPos;
in vec2 vUV;
in mat3 vTBN;

// MRT outputs
layout(location = 0) out vec4 gNormal;    // RGBA16F: world-space normal
layout(location = 1) out vec4 gAlbedo;    // RGBA8:   albedo
layout(location = 2) out vec4 gMaterial;  // RGBA8:   metallic(r), roughness(g), ao(b)

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetalRoughMap;   // R=occlusion, G=roughness, B=metallic (glTF ORM)

uniform vec3  u_AlbedoFactor;
uniform float u_MetallicFactor;
uniform float u_RoughnessFactor;

void main()
{
    vec3 albedo   = texture(u_AlbedoMap, vUV).rgb * u_AlbedoFactor;

    // Decode tangent-space normal and transform to world space
    vec3 normalTS = texture(u_NormalMap, vUV).rgb * 2.0 - 1.0;
    vec3 worldN   = normalize(vTBN * normalTS);

    // glTF ORM convention: R=occlusion, G=roughness, B=metallic
    vec3 orm      = texture(u_MetalRoughMap, vUV).rgb;
    float ao        = orm.r;
    float roughness = orm.g * u_RoughnessFactor;
    float metallic  = orm.b * u_MetallicFactor;

    gNormal   = vec4(worldN, 1.0);
    gAlbedo   = vec4(albedo, 1.0);
    gMaterial = vec4(metallic, roughness, ao, 1.0);
}
