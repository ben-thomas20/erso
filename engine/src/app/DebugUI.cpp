#include "DebugUI.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace engine {

DebugUI::DebugUI(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    // Slightly softer dark theme tweaks.
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 6.f;
    style.FrameRounding     = 4.f;
    style.GrabRounding      = 4.f;
    style.FramePadding      = {6.f, 4.f};
    style.ItemSpacing       = {8.f, 5.f};
    style.Colors[ImGuiCol_WindowBg] = {0.10f, 0.10f, 0.10f, 0.88f};
    style.Colors[ImGuiCol_Header]   = {0.25f, 0.25f, 0.30f, 1.00f};

    // Install GLFW + OpenGL3 backends.  "#version 410" matches macOS GL 4.1.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

DebugUI::~DebugUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void DebugUI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::Draw(DebugUIData& data)
{
    ImGui::SetNextWindowPos ({10.f, 10.f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({340.f, 560.f}, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Engine Debug — Phase 6")) {
        ImGui::End();
        return;
    }

    // ── Performance ───────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Sort passes by name for stable ordering.
        std::vector<std::pair<std::string,float>> sorted(
            data.gpuTimes.begin(), data.gpuTimes.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b){ return a.first < b.first; });

        float maxMs = 0.1f;
        for (const auto& [k, v] : sorted) maxMs = std::max(maxMs, v);

        for (const auto& [label, ms] : sorted) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.2f ms", ms);
            ImGui::ProgressBar(ms / maxMs, {-1.f, 0.f}, buf);
            ImGui::SameLine(0.f, 4.f);
            ImGui::TextUnformatted(label.c_str());
        }
        ImGui::Separator();
        ImGui::Text("CPU frame: %.2f ms  (%.0f fps)",
                    data.frameMs, data.frameMs > 0.f ? 1000.f / data.frameMs : 0.f);
    }

    // ── Culling ───────────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Culling", ImGuiTreeNodeFlags_DefaultOpen)) {
        const float pct = data.totalMeshCount > 0
            ? 100.f * static_cast<float>(data.culledCount)
                    / static_cast<float>(data.totalMeshCount)
            : 0.f;
        ImGui::Text("Total:    %u", data.totalMeshCount);
        ImGui::Text("Visible:  %u", data.totalMeshCount - data.culledCount);
        ImGui::Text("Culled:   %u  (%.1f%%)", data.culledCount, pct);
        ImGui::Text("Draw calls: %u", data.drawCallCount);
    }

    // ── G-Buffer previews ─────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("G-Buffer")) {
        const float sz = 70.f;
        const ImVec2 uv0{0.f, 1.f}, uv1{1.f, 0.f};  // flip Y for GL convention

        auto thumb = [&](std::uint32_t id, const char* tip) {
            ImGui::Image(static_cast<ImTextureID>(static_cast<std::uint64_t>(id)),
                         {sz, sz}, uv0, uv1);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tip);
            ImGui::SameLine();
        };

        thumb(data.gNormalTexID,   "Normal");
        thumb(data.gAlbedoTexID,   "Albedo");
        thumb(data.gMaterialTexID, "ORM");
        thumb(data.hdrTexID,       "HDR");
        ImGui::NewLine();
    }

    // ── Light inspector ───────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (data.lightDirPtr) {
            if (ImGui::DragFloat3("Direction",
                    glm::value_ptr(*data.lightDirPtr), 0.01f, -1.f, 1.f)) {
                const float len = glm::length(*data.lightDirPtr);
                if (len > 1e-4f) *data.lightDirPtr /= len;
            }
        }
        if (data.lightColorPtr)
            ImGui::ColorEdit3("Color", glm::value_ptr(*data.lightColorPtr));
        if (data.lightIntensityPtr)
            ImGui::SliderFloat("Intensity", data.lightIntensityPtr, 0.f, 20.f);
    }

    // ── Shader hot-reload ─────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("Shader Hot-Reload")) {
        if (lastReloaded_.empty()) {
            ImGui::TextDisabled("No reload yet — edit any .glsl to trigger");
        } else {
            ImGui::TextColored({0.4f, 1.f, 0.4f, 1.f},
                               "Last: %s", lastReloaded_.c_str());
        }
        ImGui::TextDisabled("Reload latency: ~1 frame");
    }

    ImGui::End();
}

void DebugUI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace engine
