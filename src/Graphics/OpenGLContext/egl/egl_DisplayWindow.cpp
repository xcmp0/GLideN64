// The display window of the Zilmar plugin API on systems without a window for
// it (Project64's headless frontend): the EGL pbuffer of EglContext at the
// windowed size of the config. Screenshots go through ReadScreen.

#include <GLideN64.h>
#include <Config.h>
#include <N64.h>
#include <RSP.h>
#include <FrameBuffer.h>
#include <Graphics/Context.h>
#include <Graphics/Parameters.h>
#include <DisplayWindow.h>
#include <Graphics/OpenGLContext/ThreadedOpenGl/opengl_Wrapper.h>
#include <stdlib.h>

using namespace opengl;

class DisplayWindowEgl : public DisplayWindow
{
public:
	DisplayWindowEgl() = default;

private:
	bool _start() override;
	void _stop() override;
	void _restart() override {}
	void _swapBuffers() override;
	void _saveScreenshot() override {}
	void _saveBufferContent(graphics::ObjectHandle /*_fbo*/, CachedTexture * /*_pTexture*/) override {}
	bool _resizeWindow() override;
	void _changeWindow() override {}
	void _readScreen(void **_pDest, long *_pWidth, long *_pHeight) override;
	void _readScreen2(void * /*_dest*/, int * /*_width*/, int * /*_height*/, int /*_front*/) override {}
	graphics::ObjectHandle _getDefaultFramebuffer() override;
};

DisplayWindow & DisplayWindow::get()
{
	static DisplayWindowEgl video;
	return video;
}

bool DisplayWindowEgl::_start()
{
	FunctionWrapper::setThreadedMode(config.video.threadedVideo);
	if (!FunctionWrapper::windowsStart())
		return false;
	return _resizeWindow();
}

void DisplayWindowEgl::_stop()
{
	FunctionWrapper::windowsStop();
}

void DisplayWindowEgl::_swapBuffers()
{
	FunctionWrapper::WaitForSwapBuffersQueued();
	FunctionWrapper::windowsSwapBuffers();
}

// The pbuffer has the size it was made with; only windowed sizes apply
bool DisplayWindowEgl::_resizeWindow()
{
	m_bFullscreen = false;
	m_screenWidth = m_width = config.video.windowedWidth;
	m_screenHeight = config.video.windowedHeight;
	m_heightOffset = 0;
	_setBufferSize();
	return true;
}

// The last frame as 24-bit BGR rows, bottom row first; free with DllCrtFree
void DisplayWindowEgl::_readScreen(void **_pDest, long *_pWidth, long *_pHeight)
{
	*_pWidth = m_width;
	*_pHeight = m_height;
	*_pDest = malloc(m_height * m_width * 3);
	if (*_pDest == nullptr)
		return;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	GLint oldMode;
	glGetIntegerv(GL_READ_BUFFER, &oldMode);
	gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, graphics::ObjectHandle::defaultFramebuffer);
	// A pbuffer is back-buffered only
	glReadBuffer(GL_BACK);
	glReadPixels(0, m_heightOffset, m_width, m_height, GL_BGR, GL_UNSIGNED_BYTE, *_pDest);
	if (graphics::BufferAttachmentParam(oldMode) == graphics::bufferAttachment::COLOR_ATTACHMENT0) {
		FrameBuffer * pBuffer = frameBufferList().getCurrent();
		if (pBuffer != nullptr)
			gfxContext.bindFramebuffer(graphics::bufferTarget::READ_FRAMEBUFFER, pBuffer->m_FBO);
	}
	glReadBuffer(oldMode);
}

graphics::ObjectHandle DisplayWindowEgl::_getDefaultFramebuffer()
{
	return graphics::ObjectHandle::null;
}
