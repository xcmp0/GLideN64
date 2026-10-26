#include "EglContext.h"
#include <Config.h>
#include <Log.h>
#include <Graphics/OpenGLContext/GLFunctions.h>
#include <string.h>

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif

namespace {
EGLDisplay s_display = EGL_NO_DISPLAY;
EGLSurface s_surface = EGL_NO_SURFACE;
EGLContext s_context = EGL_NO_CONTEXT;

EGLDisplay openDisplay()
{
	const char * extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
	PFNEGLGETPLATFORMDISPLAYEXTPROC getPlatformDisplay =
		(PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
	if (extensions != nullptr && strstr(extensions, "EGL_MESA_platform_surfaceless") != nullptr && getPlatformDisplay != nullptr) {
		EGLDisplay display = getPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr);
		if (display != EGL_NO_DISPLAY)
			return display;
	}
	return eglGetDisplay(EGL_DEFAULT_DISPLAY);
}
}

bool EglContext::start()
{
	s_display = openDisplay();
	EGLint major = 0, minor = 0;
	if (s_display == EGL_NO_DISPLAY || eglInitialize(s_display, &major, &minor) != EGL_TRUE) {
		LOG(LOG_ERROR, "EGL: no display\n");
		return false;
	}
	if (eglBindAPI(EGL_OPENGL_API) != EGL_TRUE) {
		LOG(LOG_ERROR, "EGL: no OpenGL API\n");
		stop();
		return false;
	}

	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};
	EGLConfig eglConfig;
	EGLint configCount = 0;
	if (eglChooseConfig(s_display, configAttribs, &eglConfig, 1, &configCount) != EGL_TRUE || configCount == 0) {
		LOG(LOG_ERROR, "EGL: no pbuffer config\n");
		stop();
		return false;
	}

	const EGLint surfaceAttribs[] = {
		EGL_WIDTH, (EGLint)config.video.windowedWidth,
		EGL_HEIGHT, (EGLint)config.video.windowedHeight,
		EGL_NONE
	};
	s_surface = eglCreatePbufferSurface(s_display, eglConfig, surfaceAttribs);
	if (s_surface == EGL_NO_SURFACE) {
		LOG(LOG_ERROR, "EGL: no pbuffer surface\n");
		stop();
		return false;
	}

	// A core profile, as WGL asks for; drivers give the newest version they have
	const EGLint contextAttribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 3,
		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};
	s_context = eglCreateContext(s_display, eglConfig, EGL_NO_CONTEXT, contextAttribs);
	if (s_context == EGL_NO_CONTEXT || eglMakeCurrent(s_display, s_surface, s_surface, s_context) != EGL_TRUE) {
		LOG(LOG_ERROR, "EGL: no OpenGL 3.3 core context\n");
		stop();
		return false;
	}

	initGLFunctions();
	return true;
}

void EglContext::stop()
{
	if (s_display == EGL_NO_DISPLAY)
		return;
	eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (s_context != EGL_NO_CONTEXT)
		eglDestroyContext(s_display, s_context);
	if (s_surface != EGL_NO_SURFACE)
		eglDestroySurface(s_display, s_surface);
	eglTerminate(s_display);
	s_context = EGL_NO_CONTEXT;
	s_surface = EGL_NO_SURFACE;
	s_display = EGL_NO_DISPLAY;
}

void EglContext::swapBuffers()
{
	// No effect on a pbuffer, which keeps the frame for ReadScreen
	eglSwapBuffers(s_display, s_surface);
}
