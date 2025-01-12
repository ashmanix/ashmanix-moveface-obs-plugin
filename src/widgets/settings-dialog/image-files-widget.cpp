#include "image-files-widget.h"
#include <obs-module.h>

ImageFilesWidget::ImageFilesWidget(QWidget *parent, QSharedPointer<TrackerData> tData)
	: QWidget(parent),
	  ui(new Ui::ImageFilesWidget)
{
	UNUSED_PARAMETER(tData);
	ui->setupUi(this);

	setupWidgetUI();

	connectUISignalHandlers();
}

ImageFilesWidget::~ImageFilesWidget() {}

void ImageFilesWidget::clearSelection()
{
	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
		auto it = poseImageLineEdits.find(poseEnum);
		if (it != poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value(); // For QMap
			if (lineEdit) {
				lineEdit->clear();
			}
		}
	}
	toggleVisible(false);
}

void ImageFilesWidget::toggleVisible(bool isVisible)
{
	if (isVisible) {
		ui->imageConfigWidget->setVisible(true);
		ui->noConfigLabel->setVisible(false);
	} else {
		ui->imageConfigWidget->setVisible(false);
		ui->noConfigLabel->setVisible(true);
	}
}

QMap<PoseImage, QLineEdit *> ImageFilesWidget::getposeLineEditsMap() const
{
	return poseImageLineEdits;
}

void ImageFilesWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";
	QString searchIconPath = QDir::fromNativeSeparators(baseUrl + "search.svg");
	if (QFileInfo::exists(searchIconPath)) {
		QIcon searchIcon(searchIconPath);

		ui->bodyUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesHalfOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->eyesClosedUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthClosedUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthOpenUrlBrowseToolButton->setIcon(searchIcon);
		ui->mouthSmileUrlBrowseToolButton->setIcon(searchIcon);
		ui->tongueOutUrlBrowseToolButton->setIcon(searchIcon);
	}

	QString entryClearIconPath = QDir::fromNativeSeparators(baseUrl + "entry-clear.svg");
	if (QFileInfo::exists(entryClearIconPath)) {
		QIcon entryClearIcon(entryClearIconPath);

		ui->bodyUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesHalfOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->eyesClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->mouthSmileUrlDeleteToolButton->setIcon(entryClearIcon);
		ui->tongueOutUrlDeleteToolButton->setIcon(entryClearIcon);
	}

	ui->trackingDialogScrollArea->setStyleSheet("QScrollArea {"
						    "background-color: transparent;"
						    "}"
						    "#trackingScrollAreaWidgetContents {"
						    "background-color: transparent;"
						    "}");
	ui->noConfigLabel->setStyleSheet("font-size: 20pt; padding-bottom: 40px;");
	ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));
}

//  ------------------------------------------------- Private --------------------------------------------------

void ImageFilesWidget::connectUISignalHandlers()
{
	QObject::connect(ui->bodyUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::TONGUEOUT); });

	QObject::connect(ui->bodyUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(ui->eyesOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(ui->eyesHalfOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(ui->eyesClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(ui->mouthClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(ui->mouthOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(ui->mouthSmileUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(ui->tongueOutUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::TONGUEOUT); });
}

void ImageFilesWidget::setupWidgetUI()
{
	ui->poseImageLabel->setText(obs_module_text("DialogPoseImageLabel"));
	ui->bodyUrlLabel->setText(obs_module_text("DialogBodyLabel"));
	ui->eyesOpenUrlLabel->setText(obs_module_text("DialogEyesOpenLabel"));
	ui->eyesHalfOpenUrLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	ui->eyesClosedUrlLabel->setText(obs_module_text("DialogEyesClosedLabel"));
	ui->mouthClosedUrlLabel->setText(obs_module_text("DialogMouthClosedLabel"));
	ui->mouthOpenUrlLabel->setText(obs_module_text("DialogMouthOpenLabel"));
	ui->mouthSmileUrlLabel->setText(obs_module_text("DialogMouthSmileLabel"));
	ui->tongueOutUrlLabel->setText(obs_module_text("DialogTongueOutLabel"));

	ui->bodyUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesHalfOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->eyesClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->mouthSmileUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	ui->tongueOutUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	ui->bodyUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesHalfOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->eyesClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->mouthSmileUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	ui->tongueOutUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	poseImageLineEdits[PoseImage::BODY] = ui->bodyUrlLineEdit;
	poseImageLineEdits[PoseImage::EYESOPEN] = ui->eyesOpenUrlLineEdit;
	poseImageLineEdits[PoseImage::EYESHALFOPEN] = ui->eyesHalfOpenUrlEdit;
	poseImageLineEdits[PoseImage::EYESCLOSED] = ui->eyesClosedUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHCLOSED] = ui->mouthClosedUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHOPEN] = ui->mouthOpenUrlLineEdit;
	poseImageLineEdits[PoseImage::MOUTHSMILE] = ui->mouthSmileUrlLineEdit;
	poseImageLineEdits[PoseImage::TONGUEOUT] = ui->tongueOutUrlLineEdit;

	ui->imageConfigWidget->setVisible(false);
	ui->noConfigLabel->setVisible(true);

	updateStyledUIComponents();
}

//  ---------------------------------------------- Private Slots -----------------------------------------------

void ImageFilesWidget::handleImageUrlButtonClicked(PoseImage poseEnum)
{
	QString fileName = QFileDialog::getOpenFileName(this, obs_module_text("ImageUrlFileDialogWindowTitle"),
							QString("%HOME%/Images"),
							tr("Images (*.png *.jpg *.bmp *.jpeg)"));

	emit raiseWindow();

	if (fileName.isEmpty()) {
		return;
	}

	if (!FileExists(fileName)) {
		obs_log(LOG_WARNING, QString("Image file: %1 not found!").arg(fileName).toStdString().c_str());
		return;
	}

	QLineEdit *lineEdit = poseImageLineEdits.value(poseEnum, nullptr);
	if (lineEdit) {
		lineEdit->setText(fileName);
	} else {
		obs_log(LOG_WARNING, QString("QLineEdit not found for PoseImage enum value: %1")
					     .arg(static_cast<int>(poseEnum))
					     .toStdString()
					     .c_str());
		return;
	}
	emit imageUrlSet(poseEnum, fileName);
}

void ImageFilesWidget::handleClearImageUrlButtonClicked(PoseImage poseEnum)
{
	emit raiseWindow();

	int imageIndex = static_cast<int>(poseEnum);

	if (imageIndex < 0)
		return;

	if (imageIndex >= 0 && imageIndex < static_cast<int>(poseImageLineEdits.size())) {
		poseImageLineEdits[poseEnum]->setText(QString());
	} else {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
	}
	emit imageUrlCleared(imageIndex);
}
