#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QUuid>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QDir>

#include <obs.hpp>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include "plugin-support.h"

Q_DECLARE_OPAQUE_POINTER(obs_data_t *)
constexpr const char *CONFIG = "config.json";

struct Result {
	bool success;
	QString errorMessage;
};

struct RegisterhotkeyCallbackData {
	std::function<void()> function; // Function pointer to callback function
	std::string hotkeyLogMessage;   // Message to log when hotkey is triggered
};

QString generateUniqueID();
bool fileExists(QString filePath);

void loadHotkey(int &id, const char *name, const char *description, std::function<void()> function,
		std::string buttonLogMessage, obs_data_t *savedData);

void saveHotkey(obs_data_t *sv_data, obs_hotkey_id id, const char *name);
void hotkeyCallback(void *incoming_data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);
QString getDataFolderPath();
QString getConfigFolderPath();

QString getStyleSheetDataFromFilePath(QString fileName);

#endif // UTILS_H
