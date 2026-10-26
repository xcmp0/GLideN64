#ifndef ZILMAR_SETTINGS_H
#define ZILMAR_SETTINGS_H

#include "Types.h"

// The emulator's own settings, by name, through the settings API of
// Project64 (Zilmar spec plugins). False while the emulator has not offered
// the API or has no setting of that name.
bool getEmulatorSetting(const char * _name, u32 & _value);
bool setEmulatorSetting(const char * _name, u32 _value);

#endif // ZILMAR_SETTINGS_H
