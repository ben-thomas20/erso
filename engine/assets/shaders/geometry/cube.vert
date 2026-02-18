#version 410 core
// Phase 3 — standard MeshVertex layout; UBO data from uniforms.glsl
#include "../common/uniforms.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aTangent;

out vec2 vUV;
out vec3 vNormal;
out vec3 vWorldPos;

void main()
{
    vec4 worldPos   = u_Model * vec4(aPos, 1.0);
    gl_Position     = u_ViewProjection * worldPos;
    vUV             = aUV;
    vWorldPos       = worldPos.xyz;
    // Normal in world-space using the normal matrix (upper-left 3×3 of u_NormalMatrix)
    vNormal         = normalize(mat3(u_NormalMatrix) * aNormal);
}
