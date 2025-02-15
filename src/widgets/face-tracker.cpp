#include "face-tracker.h"
#include "./settings-dialog/settings-dialog.h"
#include "../classes/poses/pose-image-data.h"
#include "../classes/poses/pose.h"
#include "../obs/moveface-image-source.h"
#include "../obs/moveface-source-private.h"

FaceTracker::FaceTracker(QWidget *parent, obs_data_t *savedData, MainWidgetDock *mDockWidget)
	: QWidget(parent),
	  m_ui(new Ui::FaceTracker)
{
	m_ui->setupUi(this);

	m_mainDockWidget = mDockWidget;
	m_trackerData = QSharedPointer<TrackerData>::create();

	if (savedData) {
		loadTrackerWidgetDataFromOBSSaveData(savedData);
	}

	if (m_trackerData->getTrackerId().size() == 0) {
		// Get a unique ID for the tracker
		m_trackerData->setTrackerId(generateUniqueID());
	}

	this->setProperty("id", m_trackerData->getTrackerId());

	setupWidgetUI();

	connectUISignalHandlers();
}

FaceTracker::~FaceTracker()
{
	m_trackerWorker = nullptr;
	delete m_ui;
}

QString FaceTracker::getTrackerID()
{
	return m_trackerData->getTrackerId();
}

void FaceTracker::setTrackerID(const QString &newId)
{
	if (!newId.isEmpty()) {
		QString oldId = m_trackerData->getTrackerId();

		// Delete saved image file using old tracker ID
		// to ensure we don't end up wth loads of unused image files
		deleteStoredImageFile(oldId);

		m_trackerData->setTrackerId(newId);
	}
	m_ui->trackerNameLabel->setText(m_trackerData->getTrackerId());
	this->setProperty("id", m_trackerData->getTrackerId());
}

TrackerData FaceTracker::getTrackerData()
{
	if (m_trackerData) {           // Always check if the pointer is not null
		return *m_trackerData; // Dereference to get a copy
	}
	return TrackerData();
}

void FaceTracker::saveTrackerWidgetDataToOBSSaveData(obs_data_t *dataObject)
{
	obs_data_set_string(dataObject, "trackerId", m_trackerData->getTrackerId().toStdString().c_str());
	obs_data_set_string(dataObject, "selectedImageSource",
			    m_trackerData->getSelectedImageSource().toStdString().c_str());

	obs_data_set_string(dataObject, "destIpAddress",
			    m_trackerData->getDestinationIpAddress().toStdString().c_str());
	obs_data_set_int(dataObject, "destPort", m_trackerData->getDestinationPort());
	obs_data_set_int(dataObject, "port", m_trackerData->getPort());
	obs_data_set_bool(dataObject, "isEnabled", m_trackerData->getIsEnabled());

	QString poseListJson = m_trackerData->poseListToJsonString();
	obs_data_set_string(dataObject, "poseList", poseListJson.toStdString().c_str());

	savePoseImageToFile();
}

void FaceTracker::loadTrackerWidgetDataFromOBSSaveData(obs_data_t *dataObject)
{

	m_trackerData->setTrackerId((char *)obs_data_get_string(dataObject, "trackerId"));
	m_trackerData->setSelectedImageSource((char *)obs_data_get_string(dataObject, "selectedImageSource"));

	m_trackerData->setDestinationIpAddress((char *)obs_data_get_string(dataObject, "destIpAddress"));
	m_trackerData->setDestinationPort(obs_data_get_int(dataObject, "destPort"));
	m_trackerData->setPort(obs_data_get_int(dataObject, "port"));
	m_trackerData->setIsEnabled(obs_data_get_bool(dataObject, "isEnabled"));

	QString poseListString = (char *)obs_data_get_string(dataObject, "poseList");
	m_trackerData->jsonStringToPoseList(poseListString);

	setTrackerData();

	loadPoseImageFromSavedFile();

	toggleEnabled(m_trackerData->getIsEnabled() ? Qt::Checked : Qt::Unchecked);
}

void FaceTracker::updateWidgetStyles()
{
	if (m_settingsDialogUi) {
		m_settingsDialogUi->updateStyledUIComponents();
	}
}

void FaceTracker::removeFromList()
{
	deleteStoredImageFile(m_trackerData->getTrackerId());
	this->deleteLater();
}

// ---------------------------------- Private -------------------------------------

void FaceTracker::setupWidgetUI()
{
	setTrackerID();

	QMenu *toolButtonMenu = new QMenu(this);
	toolButtonMenu->addAction("Settings", this, &FaceTracker::settingsActionSelected);
	toolButtonMenu->addSeparator();
	toolButtonMenu->addAction("Delete", this, &FaceTracker::deleteActionSelected);
	m_ui->menuToolButton->setMenu(toolButtonMenu);
	m_ui->menuToolButton->setPopupMode(QToolButton::InstantPopup);
	m_ui->menuToolButton->setArrowType(Qt::NoArrow);
	m_ui->menuToolButton->setText("");
	m_ui->menuToolButton->setEnabled(true);
	m_ui->menuToolButton->setToolTip(obs_module_text("TrackerSettingsButtonToolTip"));

	m_ui->errorLabel->setVisible(false);

	m_ui->mainWidgetFrame->setProperty("class", "bg-base");

	m_ui->isEnabledCheckBox->setToolTip(obs_module_text("ToggleEnabledTickBoxToolTip"));

	m_ui->connectionLabel->setFixedSize(12, 12);

	setConnected(false);
}

void FaceTracker::connectUISignalHandlers()
{
	QObject::connect(m_ui->isEnabledCheckBox, &QCheckBox::stateChanged, this, &FaceTracker::toggleEnabled);
}

void FaceTracker::setTrackerData()
{
	m_ui->trackerNameLabel->setText(m_trackerData->getTrackerId());
	m_ui->isEnabledCheckBox->setChecked(m_trackerData->getIsEnabled());
	loadPoseData();
}

void FaceTracker::loadPoseData()
{
	for (qsizetype i = 0; i < m_trackerData->getPoseList().size(); i++) {
		QSharedPointer<Pose> selectedPose = m_trackerData->getPoseAt(i);

		for (size_t j = 0; j < selectedPose->getPoseImageListSize(); ++j) {
			PoseImage poseEnum = static_cast<PoseImage>(j);
			PoseImageData *data = selectedPose->getPoseImageAt(j);

			if (!data->getImageUrl().isEmpty()) {
				data->setPixmapItem(QSharedPointer<MovablePixmapItem>(new MovablePixmapItem(poseEnum)));

				QPixmap pixmap(data->getImageUrl());
				if (!pixmap.isNull()) {
					data->getPixmapItem()->setPixmap(pixmap);
				} else {
					obs_log(LOG_ERROR, "Failed to load pixmap from URL: %s",
						data->getImageUrl().toStdString().c_str());
					data->clearPixmapItem();
					continue;
				}

				switch (poseEnum) {
				case PoseImage::BODY:
					data->setImagePosition(selectedPose->getBodyPosition().getX(),
							       selectedPose->getBodyPosition().getY());
					break;
				case PoseImage::EYESOPEN:
				case PoseImage::EYESHALFOPEN:
				case PoseImage::EYESCLOSED:
					data->setImagePosition(selectedPose->getEyesPosition().getX(),
							       selectedPose->getEyesPosition().getY());
					break;
				case PoseImage::MOUTHCLOSED:
				case PoseImage::MOUTHOPEN:
				case PoseImage::MOUTHSMILE:
				case PoseImage::TONGUEOUT:
					data->setImagePosition(selectedPose->getMouthPosition().getX(),
							       selectedPose->getMouthPosition().getY());
					break;
				default:
					break;
				}
			}
		}
	}
}

void FaceTracker::setConnected(bool isConnectedInput)
{
	m_isConnected = isConnectedInput;

	if (m_isConnected && m_ui) {
		m_ui->connectionLabel->setStyleSheet("QLabel { "
						     "  background-color:  rgb(0, 128, 0); "
						     "  border-radius: 6px; "
						     "}");
	} else {
		m_ui->connectionLabel->setStyleSheet("QLabel { "
						     "  background-color:  rgb(192, 0, 0); "
						     "  border-radius: 6px; "
						     "}");
	}
}

void FaceTracker::updateTrackerDataFromDialog(QSharedPointer<TrackerData> newData)
{
	if (newData->getDestinationIpAddress() != m_trackerData->getDestinationIpAddress() ||
	    newData->getDestinationPort() != m_trackerData->getDestinationPort() ||
	    newData->getPort() != m_trackerData->getPort() || newData->getPoseList() != m_trackerData->getPoseList()) {

		m_trackerData->setDestinationIpAddress(newData->getDestinationIpAddress());
		m_trackerData->setDestinationPort(newData->getDestinationPort());
		m_trackerData->setPort(newData->getPort());

		m_trackerData->copyListToPoseList(newData->getPoseList());

		// Renew connection
		initiateTracking();
	}

	m_trackerData->setSelectedImageSource(newData->getSelectedImageSource());

	// We set the image source alignment to center so all poses are centered on each other
	setImageSourcePositionToCenter(newData->getSelectedImageSource());

	m_mainDockWidget->updateTrackerList(m_trackerData->getTrackerId(), newData->getTrackerId());
}

// Set image source position to center
void FaceTracker::setImageSourcePositionToCenter(QString sourceName)
{
	obs_source_t *current_scene = obs_frontend_get_current_scene();
	if (!current_scene)
		return;

	obs_sceneitem_t *item =
		obs_scene_find_source(obs_scene_from_source(current_scene), sourceName.toStdString().c_str());

	if (item) {
		obs_sceneitem_set_alignment(item, OBS_ALIGN_CENTER);
		obs_sceneitem_release(item);
	}

	obs_source_release(current_scene);
}

void FaceTracker::initiateTracking()
{
	if (!m_trackerData->getIsEnabled())
		return;

	m_ui->errorLabel->setVisible(false);
	if (!m_trackerWorker) {
		m_trackerWorker = QSharedPointer<TrackerWorker>::create(m_trackerData->getPort(),
									m_trackerData->getDestinationIpAddress(),
									m_trackerData->getDestinationPort(), this);

		QObject::connect(m_trackerWorker.data(), &TrackerWorker::connectionToggle, this,
				 &FaceTracker::setConnected);

		QObject::connect(m_trackerWorker.data(), &TrackerWorker::errorOccurred, this,
				 &FaceTracker::toggleConnectionError);

		QObject::connect(m_trackerWorker.data(), &TrackerWorker::imageReady, this,
				 &FaceTracker::handleDisplayNewImage);

		// Set tracker data for worker
		m_trackerWorker->start();
	} else {
		m_trackerWorker->updateConnection(m_trackerData->getPort(), m_trackerData->getDestinationIpAddress(),
						  m_trackerData->getDestinationPort());
	}
	m_trackerWorker->updateTrackerData(m_trackerData);

	if (m_settingsDialogUi) {
		m_settingsDialogUi->updateTrackerWorker(m_trackerWorker);
	}
}

void FaceTracker::enableTimer()
{
	m_trackerData->setIsEnabled(true);
	m_ui->trackerNameLabel->setEnabled(true);
	initiateTracking();
}

void FaceTracker::disableTimer()
{
	m_trackerData->setIsEnabled(false);
	m_ui->trackerNameLabel->setEnabled(false);
	if (m_trackerWorker) {
		m_trackerWorker->stop();
		m_trackerWorker = nullptr;
		setConnected(false);
	}
}

void FaceTracker::loadPoseImageFromSavedFile()
{
	QString imageFilePath = getImageFilePath(getTrackerID());
	QImage imageFile(imageFilePath);
	if (imageFile.isNull()) {
		obs_log(LOG_WARNING, "Could not load move face image from file URL: %s",
			imageFilePath.toStdString().c_str());
	} else {
		handleDisplayNewImage(&imageFile);
	}
}

void FaceTracker::savePoseImageToFile()
{
	QString imageFilePath = getImageFilePath(getTrackerID());
	if (m_cachedPoseImage) {
		obs_log(LOG_INFO, "Saving move face image to location: %s", imageFilePath.toStdString().c_str());
		m_cachedPoseImage->save(imageFilePath);
	}
}

void FaceTracker::deleteStoredImageFile(QString imageId)
{
	if (imageId.isNull())
		return;

	QString imageFilePath = getImageFilePath(imageId);
	QFile imageFile(imageFilePath);

	if (QFileInfo::exists(imageFilePath)) {
		if (!imageFile.remove()) {
			if (imageFile.error() == QFile::PermissionsError) {
				obs_log(LOG_ERROR, "Insufficient permissions to delete file: %s",
					imageFilePath.toStdString().c_str());
			} else {
				obs_log(LOG_ERROR, "Error deleting file: %s. Error: %s",
					imageFilePath.toStdString().c_str(),
					imageFile.errorString().toStdString().c_str());
			}
		}

	} else {
		obs_log(LOG_WARNING, "Attempting to delete file but file not found: %s",
			imageFilePath.toStdString().c_str());
	}
}

QString FaceTracker::getImageFilePath(QString fileName)
{
	// First we sanitise the filename
	QRegularExpression invalidChars(R"([\\/*?":<>|])");
	QString cleanFileName = fileName;
	cleanFileName.replace(invalidChars, "");
	cleanFileName = cleanFileName.trimmed();
	cleanFileName = cleanFileName.toLower();

	QString baseDir = getConfigFolderPath() + "/images";
	QDir dir(baseDir);

	// Check if image folder exists in config folder path if not then create
	if (!fileExists(dir.filePath(""))) {
		QDir().mkpath(dir.filePath(""));
	}

	// Then we create the file path
	// QString filePath = getConfigFolderPath() + "/images/" + cleanFileName + ".PNG";
	QString filePath = dir.filePath(cleanFileName + ".PNG");
	return filePath;
}

// ------------------------------- Private Slots ----------------------------------

void FaceTracker::settingsActionSelected()
{
	if (!m_settingsDialogUi) {
		m_settingsDialogUi =
			QSharedPointer<SettingsDialog>::create(this, m_trackerData, m_mainDockWidget, m_trackerWorker);
		QObject::connect(m_settingsDialogUi.data(), &SettingsDialog::settingsUpdated, this,
				 &FaceTracker::updateTrackerDataFromDialog);
	}
	if (m_settingsDialogUi->isVisible()) {
		m_settingsDialogUi->raise();
		m_settingsDialogUi->activateWindow();
	} else {
		m_settingsDialogUi->setVisible(true);
	}
}

void FaceTracker::deleteActionSelected()
{
	emit requestDelete(m_trackerData->getTrackerId());
}

void FaceTracker::handleDisplayNewImage(QImage *image)
{
	// If we have a source set then we replace the source image with this image
	QString selectedImageSource = m_trackerData->getSelectedImageSource();
	if (selectedImageSource.isNull())
		return;

	// Cache the image pointer
	m_cachedPoseImage = QSharedPointer<QImage>::create(*image);

	// Get the source first
	std::string tempSourceName = selectedImageSource.toStdString();
	const char *sourceName = tempSourceName.c_str();
	obs_source_t *selectedSource = obs_get_source_by_name(sourceName);
	if (!selectedSource) {
		obs_log(LOG_ERROR, "Source %s not found", sourceName);
		return;
	}

	moveface_image_source *privateData = getMyPrivateData(selectedSource);
	if (!privateData) {
		obs_log(LOG_ERROR, "Private data not found for source: %s", sourceName);
		obs_source_release(selectedSource);
		return;
	}
	image_source_update_texture(privateData, image);

	obs_source_release(selectedSource);
}

void FaceTracker::toggleEnabled(int checkState)
{
	switch (checkState) {
	case Qt::Checked:
		enableTimer();
		break;

	case Qt::Unchecked:
	default:
		disableTimer();
		break;
	}
}

void FaceTracker::toggleConnectionError(bool isError)
{
	m_ui->errorLabel->setVisible(isError);
}
