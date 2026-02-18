#version 410 core
// Phase 4 — Kawase bloom (downsample + upsample)

in  vec2 vUV;
out vec4 FragColor;

uniform sampler2D uSource;
uniform float     uThreshold;
uniform bool      uIsUpsample;

void main()
{
    vec2 texelSize = 1.0 / textureSize(uSource, 0);

    if (!uIsUpsample) {
        // Threshold + downsample
        vec3 color = texture(uSource, vUV).rgb;
        float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
        FragColor = vec4(color * max(0.0, luminance - uThreshold), 1.0);
    } else {
        // 4-tap Kawase upsample
        vec3 sum = vec3(0.0);
        sum += texture(uSource, vUV + vec2(-texelSize.x,  texelSize.y)).rgb;
        sum += texture(uSource, vUV + vec2( texelSize.x,  texelSize.y)).rgb;
        sum += texture(uSource, vUV + vec2(-texelSize.x, -texelSize.y)).rgb;
        sum += texture(uSource, vUV + vec2( texelSize.x, -texelSize.y)).rgb;
        FragColor = vec4(sum * 0.25, 1.0);
    }
}
