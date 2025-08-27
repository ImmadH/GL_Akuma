#include <Windows.h>
#include <wingdi.h>
#include "opengl.h"
#include "glad/glad.h" 
//Function to take a window handle and return a opengl context

HGLRC enableGL(HWND handle, HDC* outHdc)
{
  HDC hDC = GetDC(handle);

  PIXELFORMATDESCRIPTOR pfd = {0};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cAlphaBits = 8;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pf = ChoosePixelFormat(hDC, &pfd);
  if (!pf) 
  { 
    ReleaseDC(handle, hDC); return NULL; 
  }
  if (!SetPixelFormat(hDC, pf, &pfd)) 
  { 
    ReleaseDC(handle, hDC); return NULL; 
  }

  HGLRC hRC = wglCreateContext(hDC);
  if (!hRC) 
  { 
    ReleaseDC(handle, hDC); return NULL; 
  }

  if (!wglMakeCurrent(hDC, hRC)) 
  {
    wglDeleteContext(hRC);
    ReleaseDC(handle, hDC);
    return NULL;
  }

  if(!gladLoadGL())
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(handle, hDC);
    return NULL;
  }




  *outHdc = hDC;            // keep the HDC for SwapBuffers each frame
  return hRC;
}
