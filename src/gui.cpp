#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "imgui/imgui.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>

#ifndef IMGUI_IMPL_WIN32_WNDPROC_DECLARED
#define IMGUI_IMPL_WIN32_WNDPROC_DECLARED
extern "C++" LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
#endif

extern "C" {
#include "gui.h"   
}

void gui_init(HWND hwnd, const char* glsl_version)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();  
    (void)io;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void gui_shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gui_new_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void gui_render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool gui_wndproc_handler(HWND hwnd, unsigned int msg, unsigned long wparam, long lparam)
{
    // returns LRESULT; convert to bool
    return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam) != 0;
}

bool gui_want_capture_input(void)
{
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

