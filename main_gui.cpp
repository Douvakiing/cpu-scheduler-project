// =============================================================================
// main_gui.cpp — ImGui + GLFW OpenGL3 front-end for the CPU scheduler demo
//
// File map (search for these banners to jump):
//   [1] INCLUDES & PLATFORM
//   [2] GLFW ERROR CALLBACK
//   [3] DOMAIN TYPES & SIMULATION (anonymous namespace)
//   [4] GANTT DRAWING
//   [5] WINDOWS WINDOW ICON
//   [6] MAIN — GLFW / OpenGL WINDOW
//   [7] MAIN — IMGUI SETUP & FONTS
//   [8] MAIN — APP STATE & DEFAULT DATA
//   [9] MAIN — PER-FRAME LOOP (9a sim clock … 9f OpenGL present)
//   [10] SHUTDOWN
// =============================================================================

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Scheduler.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define GL_SILENCE_DEPRECATION
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// --- [1] INCLUDES & PLATFORM -------------------------------------------------
// Standard library, ImGui, GLFW, and Windows/OpenGL quirks (NOMINMAX, etc.)
// are grouped above. Nothing here runs until main().

// --- [2] GLFW ERROR CALLBACK -------------------------------------------------
static void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static constexpr float kUiScale = 1.5f;

// --- [3] DOMAIN TYPES & SIMULATION -------------------------------------------
// Editable table rows + Gantt state; each step delegates to `Scheduler`.

namespace {

enum class Algorithm : int {
    FCFS = 0,
    SJF_Preemptive,
    SJF_NonPreemptive,
    RoundRobin,
    Priority_Preemptive,
    Priority_NonPreemptive,
};

static const char* kAlgorithmLabels[] = {
    "FCFS",
    "SJF (preemptive)",
    "SJF (non-preemptive)",
    "Round Robin",
    "Priority (preemptive)",
    "Priority (non-preemptive)",
};

struct EditableRow {
    char name[32]{};
    int arrival = 0;
    int burst = 1;
    int priority = 0;
};

struct GanttSeg {
    int t0 = 0;
    int t1 = 0;
    int proc = -1;
};

struct GuiSimulation {
    Scheduler scheduler;
    std::vector<std::string> names;
    std::vector<GanttSeg> gantt;
    Algorithm algo = Algorithm::FCFS;
    int now = 0;

    void appendGanttUnit(int t, int pid) {
        if (!gantt.empty() && gantt.back().proc == pid && gantt.back().t1 == t) {
            gantt.back().t1 = t + 1;
        } else {
            gantt.push_back({t, t + 1, pid});
        }
    }

    bool allDone() const { return scheduler.allProcessesFinished(); }

    void resetFromTable(const std::vector<EditableRow>& rows, Algorithm alg) {
        scheduler.clearProcesses();
        names.clear();
        gantt.clear();
        algo = alg;
        scheduler.setCurrentTime(0);
        now = 0;
        for (const EditableRow& row : rows) {
            if (row.burst <= 0) {
                continue;
            }
            const std::string displayName = row.name[0] ? row.name : "P";
            names.push_back(displayName);
            Process p(displayName, std::max(0, row.arrival), row.burst, row.priority, false);
            scheduler.addProcess(p);
        }
    }

    bool step(int rrQuantum) {
        if (names.empty()) {
            return false;
        }
        // if (allDone()) {
        //     return false;
        // }

        const int t = scheduler.getCurrentTime();
        Process r("IDLE", t, 0);
        switch (algo) {
        case Algorithm::FCFS:
            r = scheduler.FCFS();
            break;
        case Algorithm::SJF_Preemptive:
            r = scheduler.SJF_Premetive();
            break;
        case Algorithm::SJF_NonPreemptive:
            r = scheduler.SJF_NonPremetive();
            break;
        case Algorithm::RoundRobin:
            r = scheduler.RR(rrQuantum);
            break;
        case Algorithm::Priority_Preemptive:
            r = scheduler.Priority_Premetive();
            break;
        case Algorithm::Priority_NonPreemptive:
            r = scheduler.Priority_NonPremetive();
            break;
        default:
            return false;
        }

        int procIdx = -1;
        if (r.getName() != "IDLE") {
            const std::vector<Process>& procs = scheduler.getProcesses();
            for (size_t i = 0; i < procs.size(); ++i) {
                if (procs[i].getPid() == r.getPid()) {
                    procIdx = static_cast<int>(i);
                    break;
                }
            }
        }

        appendGanttUnit(t, procIdx);
        scheduler.advanceTime();
        now = scheduler.getCurrentTime();
        return true;
    }

    void addProcessAtCurrentTime(const std::string& displayName, int burst, int priority) {
        const int arrival = scheduler.getCurrentTime();
        names.push_back(displayName);
        Process p(displayName, arrival, std::max(1, burst), priority, false);
        scheduler.addProcess(p);
    }
};

static ImU32 procColor(int procIndex, int n) {
    if (procIndex < 0) {
        return IM_COL32(80, 80, 90, 255);
    }
    float h = (n > 0) ? (static_cast<float>(procIndex % n) / static_cast<float>(std::max(1, n))) + 0.1f * static_cast<float>(procIndex) : 0.f;
    h = h - std::floor(h);
    ImVec4 c = ImColor::HSV(h, 0.65f, 0.95f);
    return ImGui::ColorConvertFloat4ToU32(c);
}

// --- [4] GANTT DRAWING -------------------------------------------------------
// Pure ImGui draw-list code: timeline grid, per-process bars, idle strip.

// Returns pixel height of the drawn chart area (0 if nothing drawn to the draw list).
float drawGanttChart(const GuiSimulation& sim, ImDrawList* dl, ImVec2 origin, float width, float rowHeight, float maxTimeUnitWidth) {
    if (sim.names.empty()) {
        ImGui::TextDisabled("Add processes and run a simulation to see the Gantt chart.");
        return 0.f;
    }

    int tEnd = 0;
    for (const GanttSeg& s : sim.gantt) {
        tEnd = std::max(tEnd, s.t1);
    }
    tEnd = std::max(tEnd, sim.now);
    if (tEnd <= 0) {
        ImGui::TextDisabled("No timeline yet. Press Run.");
        return 0.f;
    }

    ImGui::Text("Simulation time: %d", sim.now);

    const float labelW = 100.f * (kUiScale / 2.f);
    const float chartAreaW = std::max(1.f, width - labelW);
    // Fit the full horizon into the panel; cap width per unit when the timeline is short.
    const float timeUnitWidth =
        std::min(maxTimeUnitWidth, chartAreaW / static_cast<float>(tEnd));

    const int n = static_cast<int>(sim.names.size());
    const float chartH = rowHeight * static_cast<float>(n) + rowHeight * 0.5f;

    dl->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + chartH + 24.f), IM_COL32(30, 30, 36, 255), 4.f);
    dl->AddRect(origin, ImVec2(origin.x + width, origin.y + chartH + 24.f), IM_COL32(60, 60, 70, 255), 4.f);

    for (int tick = 0; tick <= tEnd; ++tick) {
        float x = origin.x + labelW + static_cast<float>(tick) * timeUnitWidth;
        dl->AddLine(ImVec2(x, origin.y), ImVec2(x, origin.y + chartH), IM_COL32(50, 50, 58, 255), 1.f);
        if (tick < tEnd) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%d", tick);
            dl->AddText(ImVec2(x + 2.f, origin.y + chartH + 4.f), IM_COL32(180, 180, 190, 255), buf);
        }
    }

    for (int r = 0; r < n; ++r) {
        float y0 = origin.y + static_cast<float>(r) * rowHeight;
        float y1 = y0 + rowHeight - 4.f;
        dl->AddText(ImVec2(origin.x + 6.f, y0 + 4.f), IM_COL32(220, 220, 230, 255), sim.names[r].c_str());
        for (const GanttSeg& s : sim.gantt) {
            if (s.proc != r) {
                continue;
            }
            float x0 = origin.x + labelW + static_cast<float>(s.t0) * timeUnitWidth;
            float x1 = origin.x + labelW + static_cast<float>(s.t1) * timeUnitWidth;
            ImU32 col = procColor(r, n);
            dl->AddRectFilled(ImVec2(x0 + 1.f, y0 + 2.f), ImVec2(x1 - 1.f, y1), col, 2.f);
        }
    }

    float idleY = origin.y + chartH - rowHeight * 0.35f;
    for (const GanttSeg& s : sim.gantt) {
        if (s.proc != -1) {
            continue;
        }
        float x0 = origin.x + labelW + static_cast<float>(s.t0) * timeUnitWidth;
        float x1 = origin.x + labelW + static_cast<float>(s.t1) * timeUnitWidth;
        dl->AddRectFilled(ImVec2(x0 + 1.f, idleY), ImVec2(x1 - 1.f, idleY + 8.f), procColor(-1, n), 2.f);
    }
    dl->AddText(ImVec2(origin.x + 6.f, idleY - 2.f), IM_COL32(160, 160, 170, 255), "idle");

    return chartH + 24.f;
}

} // namespace

// --- [5] WINDOWS WINDOW ICON -----------------------------------------------
#if defined(_WIN32)
// Must match resources/app.rc (IDI_MAINICON = 1; Explorer uses this as the file icon).
static constexpr UINT kAppIconResourceId = 1;

// Load the same embedded ICON resource used for the .exe file icon and pass pixels to GLFW.
static void apply_window_icon_from_exe_resource(GLFWwindow* window) {
    HINSTANCE inst = GetModuleHandleW(nullptr);
    HICON hIcon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(kAppIconResourceId), IMAGE_ICON, 64, 64, 0);
    if (hIcon == nullptr) {
        hIcon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(kAppIconResourceId), IMAGE_ICON, 32, 32, 0);
    }
    if (hIcon == nullptr) {
        hIcon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(kAppIconResourceId), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    }
    if (hIcon == nullptr) {
        return;
    }

    ICONINFO ii{};
    if (GetIconInfo(hIcon, &ii) == 0) {
        DestroyIcon(hIcon);
        return;
    }

    BITMAP bm{};
    int w = 0;
    int h = 0;
    if (ii.hbmColor != nullptr) {
        GetObject(ii.hbmColor, sizeof(bm), &bm);
        w = bm.bmWidth;
        h = bm.bmHeight;
    }
    if (ii.hbmMask != nullptr) {
        DeleteObject(ii.hbmMask);
    }
    if (ii.hbmColor != nullptr) {
        DeleteObject(ii.hbmColor);
    }

    if (w <= 0 || h <= 0) {
        DestroyIcon(hIcon);
        return;
    }

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (dib == nullptr || bits == nullptr) {
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        DestroyIcon(hIcon);
        return;
    }

    std::memset(bits, 0, static_cast<size_t>(w) * static_cast<size_t>(h) * 4u);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, dib);
    DrawIconEx(hdcMem, 0, 0, hIcon, w, h, 0, nullptr, DI_NORMAL);
    SelectObject(hdcMem, oldBmp);

    std::vector<unsigned char> rgba(static_cast<size_t>(w) * static_cast<size_t>(h) * 4u);
    const auto* src = static_cast<const unsigned char*>(bits);
    for (size_t i = 0; i < rgba.size(); i += 4) {
        rgba[i + 0] = src[i + 2];
        rgba[i + 1] = src[i + 1];
        rgba[i + 2] = src[i + 0];
        rgba[i + 3] = src[i + 3];
    }

    DeleteObject(dib);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    DestroyIcon(hIcon);

    GLFWimage icon{};
    icon.width = w;
    icon.height = h;
    icon.pixels = rgba.data();
    glfwSetWindowIcon(window, 1, &icon);
}
#else
static void apply_window_icon_from_exe_resource(GLFWwindow*) {}
#endif

// --- [6] MAIN — GLFW / OPENGL WINDOW -----------------------------------------
int main(int, char**) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CPU Scheduler", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    apply_window_icon_from_exe_resource(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // --- [7] MAIN — IMGUI SETUP & FONTS --------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    int win_w = 0;
    int win_h = 0;
    int fb_w = 0;
    int fb_h = 0;
    glfwGetWindowSize(window, &win_w, &win_h);
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    const float fb_scale_x = (win_w > 0) ? static_cast<float>(fb_w) / static_cast<float>(win_w) : 1.0f;
    const float fb_scale_y = (win_h > 0) ? static_cast<float>(fb_h) / static_cast<float>(win_h) : 1.0f;
    const float raster_density = std::max(fb_scale_x, fb_scale_y);

    const float font_px_f = std::floor(13.0f * kUiScale);
    ImFontConfig font_cfg;
    font_cfg.RasterizerDensity = raster_density;
    font_cfg.SizePixels = font_px_f;

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(kUiScale);

    ImFont* main_font = nullptr;
#if defined(_WIN32)
    main_font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", font_px_f, &font_cfg);
#endif
    if (main_font == nullptr) {
        io.Fonts->AddFontDefault(&font_cfg);
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- [8] MAIN — APP STATE & DEFAULT DATA ---------------------------------
    // `table` is what the user edits; `sim` is built from it on Run/Reset.
    std::vector<EditableRow> table;
    table.push_back({});
    std::strncpy(table[0].name, "P1", sizeof(table[0].name) - 1);
    table[0].arrival = 0;
    table[0].burst = 5;
    table[0].priority = 1;
    table.push_back({});
    std::strncpy(table[1].name, "P2", sizeof(table[1].name) - 1);
    table[1].arrival = 1;
    table[1].burst = 3;
    table[1].priority = 2;
    table.push_back({});
    std::strncpy(table[2].name, "P3", sizeof(table[2].name) - 1);
    table[2].arrival = 2;
    table[2].burst = 2;
    table[2].priority = 0;

    GuiSimulation sim;
    int algoIndex = static_cast<int>(Algorithm::FCFS);
    bool simRunning = false;
    bool simPaused = false;
    bool simReset = true;
    double nextStepAt = 0.0;
    int rrQuantum = 2;
    float stepTime = 1.0f;

    // Mid-simulation "add process" modal
    bool resumeSimAfterAddDialog = false;
    char addProcNameBuf[32]{};
    int addProcBurst = 1;
    int addProcPriority = 0;
    
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    
    sim.resetFromTable(table, static_cast<Algorithm>(algoIndex));

    // --- [9] MAIN — PER-FRAME LOOP -------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        const Algorithm algo = static_cast<Algorithm>(algoIndex);
        const bool canRunAlgo = algoIndex >= 0 && algoIndex < IM_ARRAYSIZE(kAlgorithmLabels);

        // --- [9a] Simulation clock (wall time → discrete sim steps) ------------
        if (simRunning && canRunAlgo) {
            const double tWall = glfwGetTime();
            if (tWall >= nextStepAt) {
                nextStepAt = tWall + stepTime;
                if (!sim.step(rrQuantum)) {
                    simRunning = false;
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- [9b] Full-viewport main window shell ------------------------------
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin(
            "##Main",
            nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus
                | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground);

        const bool lockTable = simRunning || simPaused;

        // --- [9c] Algorithm & future RR quantum --------------------------------
        ImGui::Text("Algorithm");
        if (lockTable) {
            ImGui::BeginDisabled();
        }
        ImGui::SetNextItemWidth(280.f);
        ImGui::Combo("##algo", &algoIndex, kAlgorithmLabels, IM_ARRAYSIZE(kAlgorithmLabels));

        if (algoIndex == static_cast<int>(Algorithm::RoundRobin)) {
            ImGui::SetNextItemWidth(280.f);
            ImGui::InputInt("RR quantum", &rrQuantum);
            if (rrQuantum < 1) {
                rrQuantum = 1;
            }
        }
        if (lockTable) {
            ImGui::EndDisabled();
        }

        ImGui::Text("Processes");
        if (lockTable) {
            ImGui::BeginDisabled();
        }

        const bool showPriority =
            (algoIndex == static_cast<int>(Algorithm::Priority_Preemptive)
             || algoIndex == static_cast<int>(Algorithm::Priority_NonPreemptive));
        const int procTableCols = showPriority ? 6 : 5;

        // Widths are proportional to the main window content width (weights are relative, not absolute px).
        if (ImGui::BeginTable(
                "procs",
                procTableCols,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthStretch, 0.06f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, showPriority ? 0.28f : 0.34f);
            ImGui::TableSetupColumn("Arrival", ImGuiTableColumnFlags_WidthStretch, 0.14f);
            ImGui::TableSetupColumn("Burst", ImGuiTableColumnFlags_WidthStretch, 0.14f);
            if (showPriority) {
                ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthStretch, 0.14f);
            }
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 0.05f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < static_cast<int>(table.size()); ++i) {
                ImGui::TableNextRow();
                ImGui::PushID(i);
                int c = 0;
                ImGui::TableSetColumnIndex(c++);
                ImGui::Text("%d", i + 1);
                ImGui::TableSetColumnIndex(c++);
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##name", table[i].name, IM_ARRAYSIZE(table[i].name));
                ImGui::TableSetColumnIndex(c++);
                ImGui::InputInt("##arrival", &table[i].arrival);
                ImGui::TableSetColumnIndex(c++);
                ImGui::InputInt("##burst", &table[i].burst);
                if (showPriority) {
                    ImGui::TableSetColumnIndex(c++);
                    ImGui::InputInt("##prio", &table[i].priority);
                }
                ImGui::TableSetColumnIndex(c++);

                if (table[i].arrival < 0) {
                    table[i].arrival = 0;
                }
                if (table[i].burst < 1) {
                    table[i].burst = 1;
                }
                if (showPriority && table[i].priority < 0) {
                    table[i].priority = 0;
                }

                if (ImGui::SmallButton("Remove")) {
        
                    table.erase(table.begin() + i);
                    i--;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (lockTable) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();

        if (ImGui::Button("Add process")) {
            if (simRunning || simPaused) {
                resumeSimAfterAddDialog = simRunning;
                if (simRunning) {
                    simRunning = false;
                    simPaused = true;
                }
                std::memset(addProcNameBuf, 0, sizeof(addProcNameBuf));
                std::snprintf(addProcNameBuf, sizeof(addProcNameBuf), "P%d", static_cast<int>(table.size()) + 1);
                addProcBurst = 1;
                addProcPriority = 0;
                ImGui::OpenPopup("AddProcessDuringSim");
            } else {
                EditableRow row{};
                std::snprintf(row.name, sizeof(row.name), "P%d", static_cast<int>(table.size()) + 1);
                row.burst = 1;
                table.push_back(row);
            }
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("AddProcessDuringSim", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Arrival time is the current simulation time (not editable).");
            ImGui::Text("Current time: %d", sim.scheduler.getCurrentTime());
            ImGui::Spacing();
            ImGui::SetNextItemWidth(280.f);
            ImGui::InputText("Name", addProcNameBuf, IM_ARRAYSIZE(addProcNameBuf));
            ImGui::SetNextItemWidth(280.f);
            ImGui::InputInt("Burst", &addProcBurst);
            if (showPriority) {
                ImGui::SetNextItemWidth(280.f);
                ImGui::InputInt("Priority", &addProcPriority);
            }
            if (addProcBurst < 1) {
                addProcBurst = 1;
            }
            if (showPriority && addProcPriority < 0) {
                addProcPriority = 0;
            }
            ImGui::Separator();
            if (ImGui::Button("Add", ImVec2(120.f, 0.f))) {
                EditableRow row{};
                std::strncpy(row.name, addProcNameBuf, sizeof(row.name) - 1);
                row.name[sizeof(row.name) - 1] = '\0';
                row.arrival = sim.scheduler.getCurrentTime();
                row.burst = addProcBurst;
                row.priority = showPriority ? addProcPriority : 0;
                table.push_back(row);
                const std::string displayName = row.name[0] ? row.name : "P";
                sim.addProcessAtCurrentTime(displayName, row.burst, row.priority);
                ImGui::CloseCurrentPopup();
                simRunning = resumeSimAfterAddDialog;
                simPaused = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120.f, 0.f))) {
                ImGui::CloseCurrentPopup();
                simRunning = resumeSimAfterAddDialog;
                simPaused = false;
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();
        ImGui::Spacing();
        if (ImGui::Button("Run") && canRunAlgo) {
            if (!simRunning && !simPaused && simReset) {
                sim.resetFromTable(table, static_cast<Algorithm>(algoIndex));
                simReset = false;
            }
            simRunning = true;
            simPaused = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            if (simRunning) {
                simRunning = false;
                simPaused = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            simRunning = false;
            simPaused = false;
            simReset = true;
            sim.resetFromTable(table, static_cast<Algorithm>(algoIndex));
        }

        ImGui::SetNextItemWidth(220.f);
        ImGui::InputFloat("Step time (s per sim tick)", &stepTime, 0.05f, 0.25f, "%.2f");
        if (stepTime < 0.01f) {
            stepTime = 0.01f;
        }

        ImGui::Spacing();
        if (simRunning || simPaused) {
            ImGui::Text("Simulation time: %d", sim.now);
        }
        ImGui::Spacing();

        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        float availW = ImGui::GetContentRegionAvail().x;
        const float rowH = 32.f * (kUiScale / 2.f);
        const float maxUnitW = 100.f * (kUiScale / 2.f);
        float ganttH = drawGanttChart(sim, ImGui::GetWindowDrawList(), canvasPos, availW, rowH, maxUnitW);
        if (ganttH > 0.f) {
            ImGui::Dummy(ImVec2(availW, ganttH));
        }

        ImGui::End();

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // --- [10] SHUTDOWN --------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
