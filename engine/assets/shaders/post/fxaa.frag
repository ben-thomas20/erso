#version 410 core
// FXAA — edge-directed anti-aliasing based on luminance gradients.
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D u_Source;

#define FXAA_EDGE_THRESHOLD      (1.0 / 8.0)
#define FXAA_EDGE_THRESHOLD_MIN  (1.0 / 24.0)
#define FXAA_SEARCH_STEPS        8
#define FXAA_SEARCH_THRESHOLD    (1.0 / 4.0)
#define FXAA_SUBPIX_TRIM         (1.0 / 4.0)
#define FXAA_SUBPIX_CAP          (3.0 / 4.0)

float Luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

void main()
{
    vec2 rcpFrame = 1.0 / vec2(textureSize(u_Source, 0));

    vec3 rgbNW = texture(u_Source, vUV + vec2(-1.0, -1.0) * rcpFrame).rgb;
    vec3 rgbNE = texture(u_Source, vUV + vec2( 1.0, -1.0) * rcpFrame).rgb;
    vec3 rgbSW = texture(u_Source, vUV + vec2(-1.0,  1.0) * rcpFrame).rgb;
    vec3 rgbSE = texture(u_Source, vUV + vec2( 1.0,  1.0) * rcpFrame).rgb;
    vec3 rgbM  = texture(u_Source, vUV).rgb;

    float lumaNW = Luma(rgbNW);
    float lumaNE = Luma(rgbNE);
    float lumaSW = Luma(rgbSW);
    float lumaSE = Luma(rgbSE);
    float lumaM  = Luma(rgbM);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float lumaRange = lumaMax - lumaMin;

    // Skip smooth regions
    if (lumaRange < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD)) {
        FragColor = vec4(rgbM, 1.0);
        return;
    }

    // Compute blend direction
    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =   (lumaNW + lumaSW) - (lumaNE + lumaSE);

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * FXAA_SUBPIX_TRIM,
                          FXAA_SEARCH_THRESHOLD);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, vec2(-8.0), vec2(8.0)) * rcpFrame;

    vec3 rgbA = 0.5 * (texture(u_Source, vUV + dir * (1.0/3.0 - 0.5)).rgb +
                       texture(u_Source, vUV + dir * (2.0/3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(u_Source, vUV + dir * -0.5).rgb +
                                       texture(u_Source, vUV + dir *  0.5).rgb);
    float lumaB = Luma(rgbB);

    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        FragColor = vec4(rgbA, 1.0);
    else
        FragColor = vec4(rgbB, 1.0);
}
