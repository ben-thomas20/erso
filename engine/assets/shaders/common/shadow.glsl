// PCF shadow sampling — include in lighting pass fragment shaders.
// Expects: uniform sampler2D u_ShadowMap;

// 5×5 Poisson-disc PCF kernel — returns shadow factor in [0, 1]
// (1.0 = fully lit, 0.0 = fully shadowed).
float PCFShadow(sampler2D shadowMap, vec3 shadowCoord)
{
    // Discard fragments outside the shadow frustum
    if (any(lessThan   (shadowCoord.xy, vec2(0.0))) ||
        any(greaterThan(shadowCoord.xy, vec2(1.0))))
        return 1.0;

    float shadow    = 0.0;
    float bias      = 0.003;
    vec2  texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    // 5×5 regular grid kernel
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float closest = texture(shadowMap, shadowCoord.xy + vec2(x, y) * texelSize).r;
            shadow += (shadowCoord.z - bias) > closest ? 0.0 : 1.0;
        }
    }
    return shadow / 25.0;
}
