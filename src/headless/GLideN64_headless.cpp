// GLideN64 as a Zilmar API (Project64) plugin on systems without Windows: the
// glue that src/windows and GLideNUI give the Windows build, for the
// offscreen display of Graphics/OpenGLContext/egl and no configuration UI.
// The configuration is the defaults, rendered on the emulation thread.

#include <PluginAPI.h>
#include <GLideN64.h>
#include <Config.h>
#include <DisplayWindow.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

Config config;

void Config_LoadConfig()
{
	config.resetToDefaults();
	// The frontend reads the screen right after each frame, on its thread
	config.video.threadedVideo = 0;
	config.validate();
}

void Config_DoConfig(void * /*_parent*/)
{
}

int PluginAPI::InitiateGFX(const GFX_INFO & _gfxInfo)
{
	_initiateGFX(_gfxInfo);
	return TRUE;
}

// The directory of this shared object (data and caches live next to it, as
// on Windows)
void PluginAPI::FindPluginPath(wchar_t * _strPath)
{
	if (_strPath == nullptr)
		return;
	std::string directory(".");
	Dl_info info;
	if (dladdr((void *)&Config_LoadConfig, &info) != 0 && info.dli_fname != nullptr) {
		std::string file(info.dli_fname);
		const std::string::size_type pos = file.find_last_of('/');
		if (pos != std::string::npos)
			directory = file.substr(0, pos);
	}
	mbstowcs(_strPath, directory.c_str(), PLUGIN_PATH_SIZE);
	_strPath[PLUGIN_PATH_SIZE - 1] = L'\0';
}

void PluginAPI::GetUserDataPath(wchar_t * _strPath)
{
	FindPluginPath(_strPath);
}

void PluginAPI::GetUserCachePath(wchar_t * _strPath)
{
	FindPluginPath(_strPath);
}

void PluginAPI::DllAbout(void * /*_hParent*/)
{
}

void PluginAPI::CaptureScreen(const char * const _Directory)
{
	dwnd().setCaptureScreen(_Directory);
}

void PluginAPI::DllConfig(void * /*_hParent*/)
{
}

void PluginAPI::GetDllInfo(PLUGIN_INFO * PluginInfo)
{
	PluginInfo->Version = 0x105;
	PluginInfo->Type = PLUGIN_TYPE_VIDEO;
	snprintf(PluginInfo->Name, sizeof(PluginInfo->Name), "%s", pluginNameWithRevision);
}

void PluginAPI::DrawStatus(const char * lpString, int32_t RightAlign)
{
	const int prevPos = config.onScreenDisplay.pos;
	config.onScreenDisplay.pos = RightAlign ? Config::posTopRight : Config::posTopLeft;
	dwnd().getDrawer().showMessage(lpString, std::chrono::milliseconds(2000));
	config.onScreenDisplay.pos = prevPos;
}
