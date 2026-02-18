#version 410 core
// ACES filmic tone mapping + gamma correction.
// Composites bloom onto the HDR buffer before mapping to LDR.
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_HDR;
uniform sampler2D u_Bloom;
uniform float u_BloomStrength;  // default 0.04

// ACES fitted curve (Krzysztof Narkowicz approximation)
vec3 ACES(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdr   = texture(u_HDR,   vUV).rgb;
    vec3 bloom = texture(u_Bloom, vUV).rgb;

    vec3 color = hdr + bloom * u_BloomStrength;
    color = ACES(color);
    color = pow(color, vec3(1.0 / 2.2));  // gamma 2.2

    FragColor = vec4(color, 1.0);
}

