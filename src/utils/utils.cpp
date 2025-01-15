#include "utils.h"

QString generateUniqueID()
{
	QUuid uuid = QUuid::createUuid();
	QByteArray hash = QCryptographicHash::hash(uuid.toByteArray(), QCryptographicHash::Md5);
	return QString(hash.toHex().left(8)); // We take the first 8 characters of the hash
}

bool FileExists(QString path)
{
	QFileInfo check_file(path);
	// Check if file exists and if yes: Is it really a file and not a directory?
	if (check_file.exists() && check_file.isFile()) {
		return true;
	} else {
		return false;
	}
}

void loadHotkey(int &id, const char *name, const char *description, std::function<void()> function,
		std::string buttonLogMessage, obs_data_t *savedData = nullptr)
{

	id = (int)obs_hotkey_register_frontend(name, description, (obs_hotkey_func)hotkeyCallback,
					       new RegisterhotkeyCallbackData{function, buttonLogMessage});

	if (savedData) {
		if (id == -1)
			return;

		OBSDataArrayAutoRelease array = obs_data_get_array(savedData, name);

		obs_hotkey_load(id, array);
	}
}

void saveHotkey(obs_data_t *sv_data, obs_hotkey_id id, const char *name)
{
	if ((int)id == -1)
		return;
	OBSDataArrayAutoRelease array = obs_hotkey_save(id);
	obs_data_set_array(sv_data, name, array);
};

void hotkeyCallback(void *incoming_data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);
	if (pressed) {
		auto *hotkey_callback_data = static_cast<RegisterhotkeyCallbackData *>(incoming_data);
		obs_log(LOG_INFO, hotkey_callback_data->hotkeyLogMessage.c_str(), " due to hotkey");
		hotkey_callback_data->function();
	}
}

QString getDataFolderPath()
{
	char *file = obs_module_file(NULL);
	QString filePath = QString::fromUtf8(file);
	bfree(file);
	return filePath;
}

QString getStyleSheetDataFromFilePath(QString fileName)
{
	QString baseUrl = getDataFolderPath() + "/styles/";

	QString styleSheetPath = QDir::fromNativeSeparators(baseUrl + fileName);
	QFile stylsheetFile(styleSheetPath);
	stylsheetFile.open(QFile::ReadOnly);
	QString styleSheetData = QLatin1String(stylsheetFile.readAll());
	if (styleSheetData.isEmpty()) {
		obs_log(LOG_ERROR, "Stylesheet is empty or failed to load.");
	}
	obs_log(LOG_INFO, "File path: %s", styleSheetPath.toStdString().c_str());
	return styleSheetData;
}
