target_sources(engine PRIVATE
    # ── Core ──────────────────────────────────────────────────────────────────
    core/Log.cpp
    core/FileSystem.cpp
    core/Timer.cpp
    core/Frustum.cpp
    core/Memory/LinearAllocator.cpp

    # ── Platform ──────────────────────────────────────────────────────────────
    platform/Input.cpp
    platform/Window.cpp

    # ── Renderer backend ──────────────────────────────────────────────────────
    renderer/backend/Buffer.cpp
    renderer/backend/VertexArray.cpp
    renderer/backend/Shader.cpp
    renderer/backend/Texture.cpp
    renderer/backend/Framebuffer.cpp

    # ── Renderer debug ────────────────────────────────────────────────────────
    renderer/debug/GPUTimer.cpp
    renderer/debug/DebugRenderer.cpp

    # ── Resources ─────────────────────────────────────────────────────────────
    resources/ShaderPreprocessor.cpp
    resources/MeshLoader.cpp
    resources/MeshBuffer.cpp
    resources/ResourceManager.cpp

    # ── Renderer frontend ─────────────────────────────────────────────────────
    renderer/frontend/Renderer.cpp
    renderer/frontend/RenderQueue.cpp
    renderer/frontend/passes/ShadowPass.cpp
    renderer/frontend/passes/GeometryPass.cpp
    renderer/frontend/passes/LightingPass.cpp
    renderer/frontend/passes/PostProcessPass.cpp

    # ── Scene / ECS ───────────────────────────────────────────────────────────
    scene/systems/TransformSystem.cpp
    scene/systems/CameraSystem.cpp
    scene/systems/RenderSystem.cpp
    scene/Scene.cpp

    # ── App ───────────────────────────────────────────────────────────────────
    app/Application.cpp
    app/DebugUI.cpp
    app/main.cpp
)

# Allow #include <core/Log.hpp>, <platform/Window.hpp>, etc.
target_include_directories(engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ── stb implementation — compiled in a separate target without -Werror ────────
# stb is a third-party single-header library that does not meet our warning
# standards; isolate it so it never sees -Wall/-Wextra/-Werror.
add_library(stb_compiled STATIC stb_impl.cpp)
target_include_directories(stb_compiled PRIVATE ${stb_SOURCE_DIR})
target_link_libraries(engine PRIVATE stb_compiled)
