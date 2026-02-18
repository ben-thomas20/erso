#pragma once

// CPU-side mirrors of the GLSL std140 UBO blocks declared in uniforms.glsl.
//
// Layout rules for std140:
//   mat4        → 64 bytes, 16-byte aligned
//   vec3        → 12 bytes, but padded to 16-byte alignment (must add float padding)
//   vec2        → 8 bytes, 8-byte aligned
//   float/int   → 4 bytes, 4-byte aligned
//
// Static asserts below verify that C++ and GLSL layouts agree exactly.

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

namespace engine {

// ─── binding = 0 — updated once per frame ────────────────────────────────────
struct alignas(16) PerFrameData {
    glm::mat4 view;              //  offset   0, size 64
    glm::mat4 projection;        //  offset  64, size 64
    glm::mat4 viewProjection;    //  offset 128, size 64
    glm::mat4 invViewProjection; //  offset 192, size 64  (for depth reconstruction)
    glm::vec3 cameraPos;         //  offset 256, size 12
    float     _pad0;             //  offset 268, size  4
    glm::vec2 resolution;        //  offset 272, size  8
    float     time;              //  offset 280, size  4
    float     deltaTime;         //  offset 284, size  4
                                 //  total: 288 bytes
};
static_assert(sizeof(PerFrameData) == 288,
              "PerFrameData size mismatch — std140 alignment broken");

// ─── binding = 1 — updated per draw call ─────────────────────────────────────
struct alignas(16) PerObjectData {
    glm::mat4 model;             //  offset  0, size 64
    glm::mat4 normalMatrix;      //  offset 64, size 64  (mat4 so std140 padding is trivial)
                                 //  total: 128 bytes
};
static_assert(sizeof(PerObjectData) == 128,
              "PerObjectData size mismatch — std140 alignment broken");

// ─── binding = 2 — directional shadow light ──────────────────────────────────
struct alignas(16) ShadowData {
    glm::mat4 lightSpaceMatrix;  //  offset  0, size 64
    glm::vec3 lightDir;          //  offset 64, size 12
    float     _pad1;             //  offset 76, size  4
    glm::vec3 lightColor;        //  offset 80, size 12
    float     lightIntensity;    //  offset 92, size  4
                                 //  total: 96 bytes
};
static_assert(sizeof(ShadowData) == 96,
              "ShadowData size mismatch — std140 alignment broken");

} // namespace engine
