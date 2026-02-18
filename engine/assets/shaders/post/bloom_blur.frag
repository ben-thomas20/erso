#version 410 core
// Dual Kawase blur — single pass, configurable iteration index.
// Run 4+ times ping-pong between two half-res FBOs for a wide soft bloom.
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_Source;
uniform int u_Iteration;  // Kawase offset scale

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_Source, 0));
    float d        = float(u_Iteration) + 0.5;

    vec3 sum  = texture(u_Source, vUV + vec2(-d, -d) * texelSize).rgb;
    sum      += texture(u_Source, vUV + vec2( d, -d) * texelSize).rgb;
    sum      += texture(u_Source, vUV + vec2(-d,  d) * texelSize).rgb;
    sum      += texture(u_Source, vUV + vec2( d,  d) * texelSize).rgb;

    FragColor = vec4(sum * 0.25, 1.0);
}
