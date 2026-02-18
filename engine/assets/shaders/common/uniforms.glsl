// Shared UBO declarations — #include this in every shader.
//
// layout(binding = N) is OpenGL 4.2+ / GL_ARB_shading_language_420pack.
// Apple's GL 4.1 driver on Metal does not support that extension, so we
// omit the GLSL binding qualifiers here and set them programmatically via
// glUniformBlockBinding inside Shader::LinkProgram.

// binding = 0 — updated once per frame
layout(std140) uniform PerFrameData {
    mat4  u_View;
    mat4  u_Projection;
    mat4  u_ViewProjection;
    mat4  u_InvViewProjection;   // for world-position reconstruction from depth
    vec3  u_CameraPos;    float _pad0;
    vec2  u_Resolution;   float u_Time;  float u_DeltaTime;
};

// binding = 1 — updated per draw call
layout(std140) uniform PerObjectData {
    mat4 u_Model;
    mat4 u_NormalMatrix;   // transpose(inverse(Model)), precomputed CPU-side
};

// binding = 2 — directional shadow light
layout(std140) uniform ShadowData {
    mat4  u_LightSpaceMatrix;
    vec3  u_LightDir;      float _pad1;
    vec3  u_LightColor;    float u_LightIntensity;
};
