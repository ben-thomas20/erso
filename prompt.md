# OpenGL Graphics Engine — Cursor Build Prompt

> **How to use:** Paste this entire document into a Cursor chat (or as a `.cursorrules` / system prompt file in your project root). Work through each phase sequentially. Each phase ends with a testable milestone so you can verify correctness before proceeding.

---

## Project Identity

You are building a modular, data-oriented OpenGL 4.6 graphics engine in C++20. The engine is designed for correctness, maintainability, and extensibility first — premature optimisation is explicitly out of scope until Phase 6. Every system you build must compile cleanly with `-Wall -Wextra -Werror` and zero undefined behaviour under ASan/UBSan.

**Target stack:**
- Language: C++20
- Graphics API: OpenGL 4.6 Core Profile
- Windowing / Input: GLFW 3.4
- OpenGL loader: GLAD2 (gl 4.6 + extensions)
- Math: GLM 1.0 (`GLM_FORCE_DEPTH_ZERO_TO_ONE`, `GLM_FORCE_LEFT_HANDED` off)
- Image loading: stb_image / stb_image_write (single-header)
- Mesh loading: assimp 5.x
- Debug UI: Dear ImGui (docking branch)
- Build system: CMake 3.25+ with FetchContent for all dependencies
- Shader language: GLSL 4.60

**Non-goals for this build:** Vulkan, audio, scripting, networking, physics. Keep scope tight.

---

## Directory Layout

Create this structure before writing any source. Do not deviate from it.

```
engine/
├── CMakeLists.txt                  # Root build file
├── cmake/
│   └── FetchDependencies.cmake     # All FetchContent declarations
├── src/
│   ├── core/                       # Utilities with zero GL dependency
│   │   ├── Assert.hpp
│   │   ├── Log.hpp / Log.cpp
│   │   ├── FileSystem.hpp / .cpp
│   │   ├── Timer.hpp / .cpp
│   │   └── Memory/
│   │       ├── LinearAllocator.hpp / .cpp
│   │       └── HandlePool.hpp      # Typed slot-map for GPU resources
│   ├── platform/                   # OS / window abstraction
│   │   ├── Window.hpp / .cpp       # Wraps GLFW, exposes engine types only
│   │   └── Input.hpp / .cpp        # Engine-side key/mouse codes
│   ├── renderer/
│   │   ├── backend/                # Raw GL resource wrappers (RAII)
│   │   │   ├── Buffer.hpp / .cpp
│   │   │   ├── VertexArray.hpp / .cpp
│   │   │   ├── Shader.hpp / .cpp
│   │   │   ├── Texture.hpp / .cpp
│   │   │   └── Framebuffer.hpp / .cpp
│   │   ├── frontend/               # Render graph, passes, sorting
│   │   │   ├── RenderCommand.hpp   # POD draw-call descriptor
│   │   │   ├── RenderQueue.hpp / .cpp
│   │   │   ├── RenderPass.hpp
│   │   │   ├── passes/
│   │   │   │   ├── ShadowPass.hpp / .cpp
│   │   │   │   ├── GeometryPass.hpp / .cpp
│   │   │   │   ├── LightingPass.hpp / .cpp
│   │   │   │   └── PostProcessPass.hpp / .cpp
│   │   │   └── Renderer.hpp / .cpp # Owns all passes, drives a frame
│   │   └── debug/
│   │       ├── DebugRenderer.hpp / .cpp  # Immediate-mode lines/boxes
│   │       └── GPUTimer.hpp / .cpp
│   ├── resources/
│   │   ├── ResourceManager.hpp / .cpp
│   │   ├── MeshLoader.hpp / .cpp
│   │   ├── TextureLoader.hpp / .cpp
│   │   └── ShaderPreprocessor.hpp / .cpp  # #include + hot-reload
│   ├── scene/
│   │   ├── ecs/
│   │   │   ├── Registry.hpp        # Entity ID + component storage
│   │   │   ├── Components.hpp      # All component structs (POD)
│   │   │   └── System.hpp          # Base system interface
│   │   ├── systems/
│   │   │   ├── RenderSystem.hpp / .cpp
│   │   │   ├── TransformSystem.hpp / .cpp
│   │   │   └── CameraSystem.hpp / .cpp
│   │   └── Scene.hpp / .cpp
│   └── app/
│       ├── Application.hpp / .cpp  # Main loop, layer stack
│       └── main.cpp
├── assets/
│   └── shaders/
│       ├── common/
│       │   ├── uniforms.glsl       # Shared UBO declarations
│       │   ├── pbr.glsl            # BRDF functions
│       │   └── shadow.glsl         # PCF sampling
│       ├── shadow/
│       │   ├── shadow.vert
│       │   └── shadow.frag
│       ├── geometry/
│       │   ├── gbuffer.vert
│       │   └── gbuffer.frag
│       ├── lighting/
│       │   ├── lighting.vert       # Fullscreen quad
│       │   └── lighting.frag
│       └── post/
│           ├── tonemap.frag
│           ├── bloom.frag
│           └── fxaa.frag
└── tests/
    └── unit/                       # Catch2 unit tests per system
```

---

## Phase 0 — Bootstrap (Milestone: Triangle on Screen)

### 0.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.25)
project(Engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json for Clangd / Cursor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/FetchDependencies.cmake)

add_executable(engine)
target_compile_options(engine PRIVATE -Wall -Wextra -Werror)
add_subdirectory(src)
target_link_libraries(engine PRIVATE glfw glad glm imgui stb assimp)
```

Populate `cmake/FetchDependencies.cmake` with FetchContent blocks for every dependency. Pin each to a specific git tag (not a branch) for reproducibility.

### 0.2 Platform Layer Rules

- `Window` must own the GLFW window pointer and call `glfwTerminate` in its destructor.
- Expose `void PollEvents()`, `void SwapBuffers()`, `bool ShouldClose()`, `glm::ivec2 GetSize()`.
- Define engine-local `enum class Key` and `enum class MouseButton` — never expose GLFW constants above this file.
- Register the OpenGL debug callback inside `Window` after context creation:

```cpp
glEnable(GL_DEBUG_OUTPUT);
glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
glDebugMessageCallback(GLDebugCallback, nullptr);
glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
    GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
```

### 0.3 Core Log System

Implement a lightweight logger with severity levels (Trace, Info, Warn, Error, Fatal). `Fatal` calls `std::abort()` after printing. Use `__FILE__`, `__LINE__`, and `__func__` macros. Do not use a third-party logging library.

### 0.4 Assert Macro

```cpp
#define ENGINE_ASSERT(cond, msg) \
    do { if (!(cond)) { LOG_FATAL("Assert failed: {} | {}", #cond, msg); } } while(0)
```

**Milestone 0:** A 800×600 window opens, a hard-coded triangle renders in a default shader, the debug callback fires on any GL error, and the window closes cleanly on Escape.

---

## Phase 1 — Backend Resource Wrappers

Each wrapper follows the same contract: non-copyable, movable, RAII lifetime, stores only an `uint32_t id_` and metadata, never stores scene data.

### 1.1 Buffer

```cpp
enum class BufferTarget { Vertex, Index, Uniform, ShaderStorage };
enum class BufferUsage  { StaticDraw, DynamicDraw, StreamDraw };

class Buffer {
public:
    Buffer(BufferTarget, BufferUsage, size_t byteSize, const void* data = nullptr);
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    void Upload(size_t offset, size_t size, const void* data);
    void Bind() const;
    void BindBase(uint32_t bindingPoint) const;  // for UBOs / SSBOs
    uint32_t GetID() const { return id_; }
private:
    uint32_t id_ = 0;
    BufferTarget target_;
};
```

### 1.2 VertexArray

```cpp
struct VertexAttribute {
    uint32_t index;
    int      count;       // components (1–4)
    uint32_t type;        // GL_FLOAT, GL_INT, etc.
    bool     normalised;
    uint32_t stride;
    size_t   offset;
    uint32_t divisor = 0; // 0 = per-vertex, 1 = per-instance
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void AttachVertexBuffer(const Buffer& vbo, std::span<const VertexAttribute> attrs);
    void AttachIndexBuffer(const Buffer& ibo);
    void Bind() const;
    void Unbind() const;
private:
    uint32_t id_ = 0;
};
```

### 1.3 Shader

```cpp
class Shader {
public:
    static Shader FromFiles(const std::filesystem::path& vert,
                             const std::filesystem::path& frag,
                             const std::filesystem::path& geom = {});
    ~Shader();

    void Bind() const;

    // Uniform setters — prefer UBOs; these are convenience for one-offs
    void SetInt    (std::string_view name, int v) const;
    void SetFloat  (std::string_view name, float v) const;
    void SetVec3   (std::string_view name, glm::vec3 v) const;
    void SetMat4   (std::string_view name, const glm::mat4& m) const;
    void SetTexture(std::string_view name, int unit) const;

    bool Reload();   // Recompile from source; returns false and keeps old program on error
    bool IsValid() const { return id_ != 0; }
private:
    uint32_t id_ = 0;
    std::filesystem::path vertPath_, fragPath_, geomPath_;
    uint32_t CompileStage(uint32_t glType, const std::string& src) const;
};
```

### 1.4 Shader Preprocessor

Implement a recursive `#include "path"` resolver in `ShaderPreprocessor`. Rules:
- Paths are relative to the file containing the directive.
- Cycle detection: track a `std::set<path>` per compilation unit; throw on re-include.
- Inject `#line N "filename"` directives after each include so GLSL errors report the correct file and line.
- The preprocessor outputs a single `std::string` ready to pass to `glShaderSource`.

### 1.5 Texture

```cpp
enum class TextureFormat { R8, RG8, RGB8, RGBA8, RGB16F, RGBA16F, Depth24Stencil8 };
enum class TextureFilter  { Nearest, Linear, LinearMipmapLinear };
enum class TextureWrap    { Repeat, ClampToEdge, ClampToBorder };

class Texture {
public:
    static Texture FromFile(const std::filesystem::path&, bool genMipmaps = true);
    static Texture Create(uint32_t w, uint32_t h, TextureFormat,
                          TextureFilter min, TextureFilter mag, TextureWrap);
    ~Texture();

    void Bind(uint32_t unit) const;
    uint32_t GetID() const { return id_; }
    glm::uvec2 GetSize() const { return size_; }
private:
    uint32_t id_ = 0;
    glm::uvec2 size_{};
};
```

### 1.6 Framebuffer

```cpp
struct AttachmentSpec {
    TextureFormat format;
    TextureFilter filter = TextureFilter::Linear;
    TextureWrap   wrap   = TextureWrap::ClampToEdge;
};

class Framebuffer {
public:
    explicit Framebuffer(uint32_t width, uint32_t height,
                         std::span<const AttachmentSpec> colorAttachments,
                         bool hasDepthStencil = true);
    ~Framebuffer();

    void Bind() const;
    static void BindDefault();
    void Resize(uint32_t w, uint32_t h);
    const Texture& GetColorAttachment(uint32_t index) const;
    const Texture& GetDepthAttachment() const;
private:
    uint32_t id_ = 0;
    glm::uvec2 size_{};
    std::vector<Texture> colorAttachments_;
    Texture depthAttachment_;
    void Rebuild();
};
```

**Milestone 1:** Render a textured cube to an offscreen framebuffer and blit it to screen. Verify no GL errors.

---

## Phase 2 — Uniform Buffer Objects and Shader Pipeline

### 2.1 UBO Layout

Define these UBOs in `assets/shaders/common/uniforms.glsl`. All shaders must `#include "common/uniforms.glsl"`.

```glsl
// binding = 0 — updated once per frame
layout(std140, binding = 0) uniform PerFrameData {
    mat4 u_View;
    mat4 u_Projection;
    mat4 u_ViewProjection;
    vec3 u_CameraPos;       float _pad0;
    vec2 u_Resolution;      float u_Time; float u_DeltaTime;
};

// binding = 1 — updated per draw call (or per batch)
layout(std140, binding = 1) uniform PerObjectData {
    mat4 u_Model;
    mat4 u_NormalMatrix;    // transpose(inverse(Model)), precomputed CPU-side
};

// binding = 2 — directional shadow light
layout(std140, binding = 2) uniform ShadowData {
    mat4 u_LightSpaceMatrix;
    vec3 u_LightDir;        float _pad1;
    vec3 u_LightColor;      float u_LightIntensity;
};
```

### 2.2 UBO Manager

Create `UniformBufferCache` (part of `Renderer`) that owns one `Buffer` per binding point and exposes:

```cpp
void UploadPerFrame (const PerFrameData&);
void UploadPerObject(const PerObjectData&);
void UploadShadow   (const ShadowData&);
```

Each method calls `Buffer::Upload` and rebinds with `glBindBufferBase`. On first call, allocate with `GL_DYNAMIC_DRAW`.

**Constraint:** Direct `glUniform*` calls are forbidden inside render passes. All per-frame and per-object data flows through UBOs. Texture unit assignment is the only remaining direct uniform set.

---

## Phase 3 — ECS

### 3.1 Entity

```cpp
using EntityID = uint32_t;
constexpr EntityID INVALID_ENTITY = 0;
```

### 3.2 Components (all POD)

```cpp
struct TransformComponent {
    glm::vec3 position    = glm::vec3(0.f);
    glm::vec3 eulerAngles = glm::vec3(0.f); // degrees
    glm::vec3 scale       = glm::vec3(1.f);
    glm::mat4 worldMatrix = glm::mat4(1.f); // computed by TransformSystem
    bool      dirty       = true;
};

struct MeshComponent {
    uint32_t meshHandle;      // Index into ResourceManager mesh table
    uint32_t materialHandle;  // Index into material table
    bool     castsShadow = true;
    bool     visible     = true;
    AABB     localBounds;     // Axis-aligned in local space
};

struct CameraComponent {
    float fovY        = 60.f;
    float nearPlane   = 0.1f;
    float farPlane    = 1000.f;
    bool  isPrimary   = false;
};

struct DirectionalLightComponent {
    glm::vec3 color     = glm::vec3(1.f);
    float     intensity = 1.f;
};

struct PointLightComponent {
    glm::vec3 color     = glm::vec3(1.f);
    float     intensity = 1.f;
    float     radius    = 10.f;
};
```

### 3.3 Registry

Implement a minimal archetype-free registry backed by `std::unordered_map<EntityID, ComponentPool>` where `ComponentPool` is a type-erased container. Use `std::type_index` as the key into a per-entity component map. This is intentionally simple — correctness over cache performance in Phase 3.

Expose:

```cpp
EntityID CreateEntity();
void     DestroyEntity(EntityID);

template<typename T>
T&   AddComponent(EntityID, T component = {});

template<typename T>
T&   GetComponent(EntityID);

template<typename T>
bool HasComponent(EntityID) const;

template<typename T>
void RemoveComponent(EntityID);

// Iterate all entities that have ALL of Ts...
template<typename... Ts>
void Each(std::function<void(EntityID, Ts&...)> fn);
```

### 3.4 Systems

Implement `TransformSystem::Update(Registry&)` which recomputes `worldMatrix` for any entity whose `TransformComponent::dirty == true`, then clears the flag.

Implement `CameraSystem::Update(Registry&, glm::ivec2 viewport)` which finds the primary camera entity, builds the view and projection matrices, and writes a `PerFrameData` struct (does not upload it — that is `Renderer`'s job).

**Milestone 3:** Load a GLTF scene via assimp into the ECS. Entities are created per mesh node with correct `TransformComponent` and `MeshComponent`. The camera orbits with mouse input.

---

## Phase 4 — Render Pipeline (Deferred + Shadows)

### 4.1 Render Pass Interface

```cpp
class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void OnResize(uint32_t w, uint32_t h) = 0;
    virtual void Execute(const RenderQueue& queue,
                         const PerFrameData& frameData) = 0;
};
```

### 4.2 Shadow Pass

- Create a `Framebuffer` with a single `Depth24Stencil8` attachment at `2048×2048`.
- Compute an orthographic `LightSpaceMatrix` that fits the scene's AABB from the directional light's perspective.
- Render all shadow-casting meshes using a minimal vertex-only shader (`shadow.vert` transforms by `u_Model * LightSpaceMatrix`; fragment shader is empty).
- Store the resulting shadow map texture and the `LightSpaceMatrix` for use in the Lighting Pass.

### 4.3 Geometry Pass (G-Buffer)

Create a `Framebuffer` with the following MRT layout:

| Attachment | Format    | Contents                               |
|------------|-----------|----------------------------------------|
| Color 0    | RGBA16F   | World-space normal (xyz), unused (w)   |
| Color 1    | RGBA8     | Albedo (rgb), unused (a)               |
| Color 2    | RGBA8     | Metallic (r), Roughness (g), AO (b)    |
| Depth      | Depth24S8 | Hardware depth                         |

`gbuffer.vert` — standard MVP transform, outputs world-space position and normal, tangent-space TBN matrix, and UV.

`gbuffer.frag` — samples albedo, normal-map (TBN transform), metallic/roughness textures and writes all four outputs.

### 4.4 Lighting Pass

Bind the G-Buffer textures and shadow map. Render a fullscreen triangle (use the classic 3-vertex clip-space trick — no vertex buffer needed). In `lighting.frag`:

1. Reconstruct world-space position from depth + inverse VP.
2. Sample G-Buffer attachments.
3. Evaluate the Cook-Torrance BRDF (`pbr.glsl` common include): direct lighting from the directional light, PCF soft shadows (5×5 kernel in `shadow.glsl`).
4. Add a simple hemisphere ambient term.
5. Output HDR colour to a `RGB16F` framebuffer.

### 4.5 Post-Processing Chain

Each effect is a fullscreen pass reading from the previous pass's output texture.

**Bloom:** Two-pass Kawase blur (downsample + upsample). Threshold pixels above luminance 1.0, blur them, add back to the HDR image.

**Tone Mapping:** ACES filmic curve. Output to `RGBA8` LDR framebuffer.

**FXAA:** Standard FXAA 3.11 (single-pass). Apply after tone mapping. Output to swapchain.

### 4.6 Draw Call Sorting

`RenderQueue` collects `RenderCommand` structs before any pass executes:

```cpp
struct RenderCommand {
    uint32_t vaoID;
    uint32_t indexCount;
    uint32_t baseIndex;
    uint32_t materialHandle;
    uint32_t shaderID;
    glm::mat4 modelMatrix;
    float     distanceToCamera;  // for transparent sorting
    bool      transparent;
};
```

Sort opaque commands front-to-back (minimise overdraw), then sort transparent commands back-to-front.

**Milestone 4:** A PBR-lit GLTF model with a directional light, soft shadows, bloom, and FXAA renders at full screen.

---

## Phase 5 — Resource Management and Hot Reload

### 5.1 Resource Manager

```cpp
class ResourceManager {
public:
    MeshHandle   LoadMesh   (const std::filesystem::path&);
    TextureHandle LoadTexture(const std::filesystem::path&, bool sRGB = true);
    MaterialHandle CreateMaterial(MaterialDesc);

    const GPUMesh&     GetMesh    (MeshHandle) const;
    const Texture&     GetTexture (TextureHandle) const;
    const Material&    GetMaterial(MaterialHandle) const;

    // Call once per frame — reloads any shaders whose source has changed on disk
    void PollShaderReload();
private:
    std::unordered_map<std::string, MeshHandle>    meshCache_;
    std::unordered_map<std::string, TextureHandle> textureCache_;
    // ...
};
```

Resources are identified by canonical absolute path. Requesting the same path twice returns the cached handle.

### 5.2 Hot Shader Reload

- Store the `std::filesystem::file_time_type` for every `.glsl` file in use at load time.
- `PollShaderReload()` iterates all tracked files, checks `last_write_time`, and calls `Shader::Reload()` on any that have changed.
- On reload failure (GLSL compile error), log the error and keep the existing compiled program. The engine must not crash on a shader typo.

### 5.3 Mesh GPU Upload

Combine all static mesh geometry into a single large VBO and IBO using a bump allocator. `GPUMesh` stores `baseVertex`, `baseIndex`, `indexCount`, and `localBounds` (AABB).

---

## Phase 6 — Frustum Culling and Debug Tooling

### 6.1 Frustum

```cpp
struct Frustum {
    std::array<glm::vec4, 6> planes; // Each vec4 is (normal.xyz, d)

    static Frustum FromViewProjection(const glm::mat4& vp);
    bool ContainsSphere(glm::vec3 center, float radius) const;
    bool ContainsAABB  (const AABB&, const glm::mat4& model) const;
};
```

`RenderSystem::GatherCommands` calls `ContainsAABB` before enqueuing any `RenderCommand`. Log the cull ratio (culled / total) as a debug stat.

### 6.2 GPU Timer

```cpp
class GPUTimer {
public:
    void Begin(std::string_view label);
    void End  (std::string_view label);
    // Call two frames later (GPU queries are async)
    std::unordered_map<std::string, float> CollectResults();
private:
    // Double-buffered query objects
};
```

Wrap each render pass in a `GPUTimer` Begin/End pair.

### 6.3 Debug Renderer

Immediate-mode API (no persistent scene state):

```cpp
void DrawLine  (glm::vec3 a, glm::vec3 b, glm::vec4 color);
void DrawAABB  (const AABB&, const glm::mat4& transform, glm::vec4 color);
void DrawSphere(glm::vec3 center, float radius, glm::vec4 color, int segments = 16);
void DrawFrustum(const Frustum&, glm::vec4 color);
void FlushAndClear(); // Called once per frame after all passes
```

Internally batches all lines into a single VBO update and one `glDrawArrays(GL_LINES, ...)` call.

### 6.4 ImGui Integration

- Hook ImGui into the GLFW/OpenGL backend using the official `imgui_impl_glfw` and `imgui_impl_opengl3` backends.
- Expose a `DebugUI::Draw()` method called at the end of each frame that shows:
  - Per-pass GPU timers (bar graph)
  - Draw call count, cull percentage
  - G-Buffer attachment previews (small quads)
  - Light inspector (direction, colour, intensity sliders — live update)
  - Shader hot-reload status indicator

**Milestone 6:** The scene renders at target frame rate with frustum culling visually confirmable via debug boxes. GPU timers visible in the ImGui overlay. Shader source can be edited in a text editor and the change reflects in the running engine within one second.

---

## Coding Standards

Follow these rules throughout every phase without exception.

**Ownership:** Every GPU resource is owned by exactly one C++ object. Passing raw GL IDs across system boundaries is forbidden — pass const references to wrapper objects or handles.

**Error handling:** Use `ENGINE_ASSERT` for programmer errors (wrong arguments, violated preconditions). Use `std::optional` or a `Result<T, Error>` type for recoverable failures (file not found, shader compile error). Do not use exceptions.

**Headers:** Every header is self-contained and includes what it uses. No forward declarations across module boundaries. Use `#pragma once`.

**Magic numbers:** No raw GL enum values in `.cpp` files outside `renderer/backend/`. All GL constants are wrapped behind the backend API.

**`std::string_view` vs `std::string`:** Use `string_view` for read-only string parameters. Store `std::string` or `std::filesystem::path` as members.

**Const correctness:** Mark all non-mutating methods `const`. Mark local variables `const` by default.

**Move semantics:** All resource wrappers must implement a move constructor and move assignment operator. The moved-from object must have `id_ = 0` so its destructor is a no-op.

---

## Build and Test Instructions

### Configure and Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"
cmake --build build -j$(nproc)
./build/engine
```

### Running With Sanitizers

Always develop with ASan + UBSan enabled. Every phase must pass with zero sanitizer errors before moving to the next phase.

### Unit Test Stubs

Create a `tests/unit/` directory with Catch2 tests for at minimum:
- `test_frustum.cpp` — sphere/AABB containment with known values
- `test_ecs.cpp` — create, query, destroy entities and components
- `test_shader_preprocessor.cpp` — include resolution, cycle detection
- `test_linear_allocator.cpp` — allocation, reset, overflow detection

---

## Iterative Implementation Order

Work strictly in this order. Do not start a phase until the milestone for the previous phase passes.

1. Phase 0 → triangle on screen
2. Phase 1 → textured cube to FBO
3. Phase 2 → UBO pipeline, camera moves
4. Phase 3 → GLTF scene in ECS, orbit camera
5. Phase 4 → full deferred PBR with shadows and post-FX
6. Phase 5 → asset caching, hot reload
7. Phase 6 → culling, GPU timers, ImGui overlay

At each phase, ask yourself: *could this layer be replaced without modifying any layer above it?* If the answer is no, the abstraction boundary is wrong. Fix it before continuing.