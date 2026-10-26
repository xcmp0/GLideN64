// The settings API of Project64 (Project64-core/Plugins/Plugin.h): after
// loading a plugin, the emulator hands it functions to read and write its
// settings through these exports.

#ifdef OS_WINDOWS
# include <windows.h>
#else
# include "winlnxdefs.h"
#endif // OS_WINDOWS

#include "PluginAPI.h"
#include "ZilmarSettings.h"

typedef struct
{
	u32 dwSize;
	s32 DefaultStartRange;
	s32 SettingStartRange;
	s32 MaximumSettings;
	s32 NoDefault;
	s32 DefaultLocation;
	void * handle;
	u32 (*GetSetting)(void * handle, s32 ID);
	const char * (*GetSettingSz)(void * handle, s32 ID, char * Buffer, s32 BufferLen);
	void (*SetSetting)(void * handle, s32 ID, u32 Value);
	void (*SetSettingSz)(void * handle, s32 ID, const char * Value);
	void (*RegisterSetting)(void * handle, s32 ID, s32 DefaultID, s32 Type, s32 Location,
		const char * Category, const char * DefaultStr, u32 Value);
	void (*UseUnregisteredSetting)(s32 ID);
} PLUGIN_SETTINGS;

typedef struct
{
	u32 (*FindSystemSettingId)(void * handle, const char * Name);
} PLUGIN_SETTINGS2;

typedef struct
{
	void (*FlushSettings)(void * handle);
} PLUGIN_SETTINGS3;

static PLUGIN_SETTINGS g_settings = {};
static PLUGIN_SETTINGS2 g_settings2 = {};
static PLUGIN_SETTINGS3 g_settings3 = {};

static u32 findSetting(const char * _name)
{
	if (g_settings.handle == nullptr || g_settings2.FindSystemSettingId == nullptr)
		return 0;
	return g_settings2.FindSystemSettingId(g_settings.handle, _name);
}

bool getEmulatorSetting(const char * _name, u32 & _value)
{
	const u32 id = findSetting(_name);
	if (id == 0 || g_settings.GetSetting == nullptr)
		return false;
	_value = g_settings.GetSetting(g_settings.handle, id);
	return true;
}

bool setEmulatorSetting(const char * _name, u32 _value)
{
	const u32 id = findSetting(_name);
	if (id == 0 || g_settings.SetSetting == nullptr)
		return false;
	g_settings.SetSetting(g_settings.handle, id, _value);
	if (g_settings3.FlushSettings != nullptr)
		g_settings3.FlushSettings(g_settings.handle);
	return true;
}

extern "C" {

EXPORT void CALL SetSettingInfo(PLUGIN_SETTINGS * _info)
{
	g_settings = *_info;
}

EXPORT void CALL SetSettingInfo2(PLUGIN_SETTINGS2 * _info)
{
	g_settings2 = *_info;
}

EXPORT void CALL SetSettingInfo3(PLUGIN_SETTINGS3 * _info)
{
	g_settings3 = *_info;
}

}
