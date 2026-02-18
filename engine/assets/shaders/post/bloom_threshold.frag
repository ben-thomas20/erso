#version 410 core
// Extract pixels above the luminance threshold for the bloom effect.
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_HDR;
uniform float u_Threshold;  // default 1.0

float Luminance(vec3 c) { return dot(c, vec3(0.2126, 0.7152, 0.0722)); }

void main()
{
    vec3 color = texture(u_HDR, vUV).rgb;
    float lum  = Luminance(color);
    // Soft knee to avoid hard cut-off
    float ramp  = clamp(lum - u_Threshold + 0.5, 0.0, 1.0);
    FragColor   = vec4(color * ramp, 1.0);
}
