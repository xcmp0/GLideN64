#pragma once

// The OpenGL context of the Zilmar plugin API on systems without WGL: an EGL
// context with an offscreen (pbuffer) surface, on a display that needs no
// window system when the driver offers one (EGL_MESA_platform_surfaceless).
// The surface is the default framebuffer, at the windowed size of the config.
class EglContext
{
public:
	static bool start();
	static void stop();
	static void swapBuffers();
};

// The Zilmar API's context commands (ThreadedOpenGl) call WindowsWGL
typedef EglContext WindowsWGL;
