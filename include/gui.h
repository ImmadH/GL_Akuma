#pragma once
#include <windows.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void gui_init(HWND hwnd, const char* glsl_version);   // e.g. "#version 460"
void gui_shutdown(void);
void gui_new_frame(void);
void gui_render(void);
bool gui_wndproc_handler(HWND hwnd, unsigned int msg, unsigned long wparam, long lparam);
bool gui_want_capture_input(void);


#ifdef __cplusplus
}
#endif
