#version 410 core
#include "../common/uniforms.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aTangent;

out vec3 vWorldPos;
out vec2 vUV;
out mat3 vTBN;   // tangent→world transform for normal-map decoding

void main()
{
    vec4 worldPos = u_Model * vec4(aPos, 1.0);
    gl_Position   = u_ViewProjection * worldPos;
    vWorldPos     = worldPos.xyz;
    vUV           = aUV;

    // Build TBN in world space using the pre-computed normal matrix.
    vec3 N = normalize(mat3(u_NormalMatrix) * aNormal);
    vec3 T = normalize(mat3(u_NormalMatrix) * aTangent);
    T = normalize(T - dot(T, N) * N);   // Gram-Schmidt re-orthogonalise
    vec3 B = cross(N, T);
    vTBN = mat3(T, B, N);
}
