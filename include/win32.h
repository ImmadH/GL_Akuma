
#include <Windows.h>
#include <mmeapi.h>
#include <winuser.h>
#include <wingdi.h>
#include "opengl.h"
#include "glad/glad.h"

typedef struct Window 
{
  HWND handle;
  HINSTANCE hInstance;
  int width;
  int height;
  const wchar_t* title;
  const wchar_t* className;
  HDC hDC;
  HGLRC openglContext;
} Window;


const wchar_t* getClassName(void);
void register_class(void);
Window* create_window(void);





