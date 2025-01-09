#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QUuid>
#include <QCryptographicHash>
#include <QFileInfo>

#include <obs.hpp>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "plugin-support.h"

Q_DECLARE_OPAQUE_POINTER(obs_data_t *)
#define CONFIG "config.json"

struct Result {
	bool success;
	QString errorMessage;
};


struct RegisterHotkeyCallbackData {
	std::function<void()> function; // Function pointer to callback function
	std::string hotkeyLogMessage;   // Message to log when hotkey is triggered
};

void LoadHotkey(int &id, const char *name, const char *description, std::function<void()> function,
		std::string buttonLogMessage, obs_data_t *savedData);

void SaveHotkey(obs_data_t *sv_data, obs_hotkey_id id, const char *name);
void HotkeyCallback(void *incoming_data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);
QString GetDataFolderPath();

QString GenerateUniqueID();
bool FileExists(QString filePath);

#endif // UTILS_H
