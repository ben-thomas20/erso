#version 410 core
// Depth-only pass: transform vertices into light-clip space.
#include "../common/uniforms.glsl"

layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = u_LightSpaceMatrix * u_Model * vec4(aPos, 1.0);
}
