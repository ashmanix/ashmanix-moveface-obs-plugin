#ifndef SETTINGSWIDGETINTERFACE_H
#define SETTINGSWIDGETINTERFACE_H

#include <QSharedPointer>

// Forward declaration
class Pose;

class SettingsWidgetInterface {
public:
	SettingsWidgetInterface() = default;
	virtual ~SettingsWidgetInterface() = default;
	virtual void setData(QSharedPointer<Pose> pose) = 0;
	virtual void clearSelection() = 0;
	virtual void toggleVisible(bool isVisible) = 0;
};

#endif // SETTINGSWIDGETINTERFACE_H
