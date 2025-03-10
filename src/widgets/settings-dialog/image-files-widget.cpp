#include "image-files-widget.h"
#include <obs-module.h>

ImageFilesWidget::ImageFilesWidget(QWidget *parent, QSharedPointer<Pose> pose)
	: QWidget(parent),
	  m_ui(new Ui::ImageFilesWidget),
	  m_pose(pose)
{
	m_ui->setupUi(this);

	if (pose)
		setData(pose);

	setupWidgetUI();

	connectUISignalHandlers();
}

ImageFilesWidget::~ImageFilesWidget() {}

void ImageFilesWidget::clearSelection()
{
	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		PoseImage poseEnum = static_cast<PoseImage>(i);
		auto it = m_poseImageLineEdits.find(poseEnum);
		if (it != m_poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value(); // For QMap
			if (lineEdit) {
				lineEdit->clear();
			}
		}
	}
}

void ImageFilesWidget::toggleVisible(bool isVisible)
{
	if (isVisible) {
		m_ui->imageConfigWidget->setVisible(true);
		m_ui->noConfigLabel->setVisible(false);
	} else {
		m_ui->imageConfigWidget->setVisible(false);
		m_ui->noConfigLabel->setVisible(true);
	}
}

QMap<PoseImage, QLineEdit *> ImageFilesWidget::getposeLineEditsMap() const
{
	return m_poseImageLineEdits;
}

void ImageFilesWidget::updateStyledUIComponents()
{
	QString baseUrl = obs_frontend_is_theme_dark() ? getDataFolderPath() + "/icons/dark/"
						       : getDataFolderPath() + "/icons/light/";
	QString searchIconPath = QDir::fromNativeSeparators(baseUrl + "search.svg");
	if (QFileInfo::exists(searchIconPath)) {
		QIcon searchIcon(searchIconPath);

		m_ui->bodyUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->eyesOpenUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->eyesHalfOpenUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->eyesClosedUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->mouthClosedUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->mouthOpenUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->mouthSmileUrlBrowseToolButton->setIcon(searchIcon);
		m_ui->tongueOutUrlBrowseToolButton->setIcon(searchIcon);
	}

	QString entryClearIconPath = QDir::fromNativeSeparators(baseUrl + "entry-clear.svg");
	if (QFileInfo::exists(entryClearIconPath)) {
		QIcon entryClearIcon(entryClearIconPath);

		m_ui->bodyUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->eyesOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->eyesHalfOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->eyesClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->mouthClosedUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->mouthOpenUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->mouthSmileUrlDeleteToolButton->setIcon(entryClearIcon);
		m_ui->tongueOutUrlDeleteToolButton->setIcon(entryClearIcon);
	}

	QString bodyIconPath = QDir::fromNativeSeparators(baseUrl + "body.svg");
	if (QFileInfo::exists(bodyIconPath)) {
		QIcon bodyIcon(bodyIconPath);
		QPixmap bodyIconPixmap = bodyIcon.pixmap(16, 16);
		m_ui->bodyUrlLabel->setPixmap(bodyIconPixmap);
	}

	QString eyeOpenIconPath = QDir::fromNativeSeparators(baseUrl + "eye-open.svg");
	if (QFileInfo::exists(eyeOpenIconPath)) {
		QIcon eyeOpenIcon(eyeOpenIconPath);
		QPixmap eyeOpenIconPixmap = eyeOpenIcon.pixmap(16, 16);
		m_ui->eyesOpenUrlLabel->setPixmap(eyeOpenIconPixmap);
	}

	QString eyeHalfOpenIconPath = QDir::fromNativeSeparators(baseUrl + "eye-half-open.svg");
	if (QFileInfo::exists(eyeHalfOpenIconPath)) {
		QIcon eyeHalfOpenIcon(eyeHalfOpenIconPath);
		QPixmap eyeHalfOpenIconPixmap = eyeHalfOpenIcon.pixmap(16, 16);
		m_ui->eyesHalfOpenUrLabel->setPixmap(eyeHalfOpenIconPixmap);
	}

	QString eyeClosedIconPath = QDir::fromNativeSeparators(baseUrl + "eye-closed.svg");
	if (QFileInfo::exists(eyeClosedIconPath)) {
		QIcon eyeClosedIcon(eyeClosedIconPath);
		QPixmap eyeClosedIconPixmap = eyeClosedIcon.pixmap(16, 16);
		m_ui->eyesClosedUrlLabel->setPixmap(eyeClosedIconPixmap);
	}

	QString mouthOpenIconPath = QDir::fromNativeSeparators(baseUrl + "mouth-open.svg");
	if (QFileInfo::exists(mouthOpenIconPath)) {
		QIcon mouthOpenIcon(mouthOpenIconPath);
		QPixmap mouthOpenIconPixmap = mouthOpenIcon.pixmap(16, 16);
		m_ui->mouthOpenUrlLabel->setPixmap(mouthOpenIconPixmap);
	}

	QString mouthClosedIconPath = QDir::fromNativeSeparators(baseUrl + "mouth-closed.svg");
	if (QFileInfo::exists(mouthClosedIconPath)) {
		QIcon mouthClosedIcon(mouthClosedIconPath);
		QPixmap mouthClosedIconPixmap = mouthClosedIcon.pixmap(16, 16);
		m_ui->mouthClosedUrlLabel->setPixmap(mouthClosedIconPixmap);
	}

	QString mouthSmileIconPath = QDir::fromNativeSeparators(baseUrl + "smile.svg");
	if (QFileInfo::exists(mouthSmileIconPath)) {
		QIcon mouthSmileIcon(mouthSmileIconPath);
		QPixmap mouthSmileIconPixmap = mouthSmileIcon.pixmap(16, 16);
		m_ui->mouthSmileUrlLabel->setPixmap(mouthSmileIconPixmap);
	}

	QString tongueOutIconPath = QDir::fromNativeSeparators(baseUrl + "tongue-out.svg");
	if (QFileInfo::exists(tongueOutIconPath)) {
		QIcon tongueOutIcon(tongueOutIconPath);
		QPixmap tongueOutIconPixmap = tongueOutIcon.pixmap(16, 16);
		m_ui->tongueOutUrlLabel->setPixmap(tongueOutIconPixmap);
	}

	m_ui->noConfigLabel->setText(obs_module_text("DialogNoConfigMessage"));
}

void ImageFilesWidget::setData(QSharedPointer<Pose> in_pose)
{
	clearSelection();

	if (in_pose)
		m_pose = in_pose;

	if (!m_pose)
		return obs_log(LOG_WARNING, "No pose data found when loading face settings!");

	for (size_t i = 0; i < static_cast<size_t>(PoseImage::COUNT); ++i) {
		auto poseEnum = static_cast<PoseImage>(i);
		auto it = m_poseImageLineEdits.find(poseEnum);
		if (it != m_poseImageLineEdits.end()) {
			QLineEdit *lineEdit = it.value();
			QString fileName = m_pose->getPoseImageData(poseEnum)->getImageUrl();
			if (lineEdit) {
				lineEdit->setText(fileName);
			}
		}
	}
}

//  ------------------------------------------------- Private --------------------------------------------------

void ImageFilesWidget::connectUISignalHandlers()
{
	QObject::connect(m_ui->bodyUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(m_ui->eyesOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(m_ui->eyesHalfOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(m_ui->eyesClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(m_ui->mouthClosedUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(m_ui->mouthOpenUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(m_ui->mouthSmileUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(m_ui->tongueOutUrlBrowseToolButton, &QToolButton::clicked, this,
			 [this]() { handleImageUrlButtonClicked(PoseImage::TONGUEOUT); });

	QObject::connect(m_ui->bodyUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::BODY); });
	QObject::connect(m_ui->eyesOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESOPEN); });
	QObject::connect(m_ui->eyesHalfOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESHALFOPEN); });
	QObject::connect(m_ui->eyesClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::EYESCLOSED); });
	QObject::connect(m_ui->mouthClosedUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHCLOSED); });
	QObject::connect(m_ui->mouthOpenUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHOPEN); });
	QObject::connect(m_ui->mouthSmileUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::MOUTHSMILE); });
	QObject::connect(m_ui->tongueOutUrlDeleteToolButton, &QToolButton::clicked, this,
			 [this]() { handleClearImageUrlButtonClicked(PoseImage::TONGUEOUT); });
}

void ImageFilesWidget::setupWidgetUI()
{
	m_ui->titleLabel->setText(obs_module_text("DialogPoseImageLabel"));
	m_ui->titleLabel->setToolTip(obs_module_text("DialogPoseImageLabelToolTip"));

	m_ui->bodyUrlLabel->setText(obs_module_text("DialogBodyLabel"));
	m_ui->eyesOpenUrlLabel->setText(obs_module_text("DialogEyesOpenLabel"));
	m_ui->eyesHalfOpenUrLabel->setText(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesClosedUrlLabel->setText(obs_module_text("DialogEyesClosedLabel"));
	m_ui->mouthClosedUrlLabel->setText(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenUrlLabel->setText(obs_module_text("DialogMouthOpenLabel"));
	m_ui->mouthSmileUrlLabel->setText(obs_module_text("DialogMouthSmileLabel"));
	m_ui->tongueOutUrlLabel->setText(obs_module_text("DialogTongueOutLabel"));

	m_ui->bodyUrlLabel->setToolTip(obs_module_text("DialogBodyLabel"));
	m_ui->eyesOpenUrlLabel->setToolTip(obs_module_text("DialogEyesOpenLabel"));
	m_ui->eyesHalfOpenUrLabel->setToolTip(obs_module_text("DialogEyesHalfOpenLabel"));
	m_ui->eyesClosedUrlLabel->setToolTip(obs_module_text("DialogEyesClosedLabel"));
	m_ui->mouthClosedUrlLabel->setToolTip(obs_module_text("DialogMouthClosedLabel"));
	m_ui->mouthOpenUrlLabel->setToolTip(obs_module_text("DialogMouthOpenLabel"));
	m_ui->mouthSmileUrlLabel->setToolTip(obs_module_text("DialogMouthSmileLabel"));
	m_ui->tongueOutUrlLabel->setToolTip(obs_module_text("DialogTongueOutLabel"));

	m_ui->bodyUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->eyesOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->eyesHalfOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->eyesClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->mouthClosedUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->mouthOpenUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->mouthSmileUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));
	m_ui->tongueOutUrlBrowseToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	m_ui->bodyUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->eyesOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->eyesHalfOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->eyesClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->mouthClosedUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->mouthOpenUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->mouthSmileUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlClearButtonToolTip"));
	m_ui->tongueOutUrlDeleteToolButton->setToolTip(obs_module_text("DialogImageUrlBrowseButtonToolTip"));

	// Initialize the mapping between PoseImage enums and QLineEdit pointers
	m_poseImageLineEdits[PoseImage::BODY] = m_ui->bodyUrlLineEdit;
	m_poseImageLineEdits[PoseImage::EYESOPEN] = m_ui->eyesOpenUrlLineEdit;
	m_poseImageLineEdits[PoseImage::EYESHALFOPEN] = m_ui->eyesHalfOpenUrlEdit;
	m_poseImageLineEdits[PoseImage::EYESCLOSED] = m_ui->eyesClosedUrlLineEdit;
	m_poseImageLineEdits[PoseImage::MOUTHCLOSED] = m_ui->mouthClosedUrlLineEdit;
	m_poseImageLineEdits[PoseImage::MOUTHOPEN] = m_ui->mouthOpenUrlLineEdit;
	m_poseImageLineEdits[PoseImage::MOUTHSMILE] = m_ui->mouthSmileUrlLineEdit;
	m_poseImageLineEdits[PoseImage::TONGUEOUT] = m_ui->tongueOutUrlLineEdit;

	m_ui->imageConfigWidget->setVisible(false);
	m_ui->noConfigLabel->setVisible(true);

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

	if (!fileExists(fileName)) {
		obs_log(LOG_WARNING, QString("Image file: %1 not found!").arg(fileName).toStdString().c_str());
		return;
	}

	QLineEdit *lineEdit = m_poseImageLineEdits.value(poseEnum, nullptr);
	if (lineEdit) {
		lineEdit->setText(fileName);
	} else {
		obs_log(LOG_WARNING, QString("QLineEdit not found for PoseImage enum value: %1")
					     .arg(static_cast<int>(poseEnum))
					     .toStdString()
					     .c_str());
		return;
	}

	auto imageIndex = static_cast<int>(poseEnum);
	m_pose->getPoseImageAt(imageIndex)->setImageUrl(fileName);

	emit imageUrlSet(poseEnum, fileName);
}

void ImageFilesWidget::handleClearImageUrlButtonClicked(PoseImage poseEnum)
{
	emit raiseWindow();

	int imageIndex = static_cast<int>(poseEnum);

	if (imageIndex < 0)
		return;

	if (m_pose && imageIndex < static_cast<int>(m_poseImageLineEdits.size())) {
		m_poseImageLineEdits[poseEnum]->setText(QString());
		PoseImageData *poseData = m_pose->getPoseImageAt(imageIndex);
		poseData->setImageUrl(QString());
	} else {
		obs_log(LOG_WARNING, "Invalid PoseImage enum value.");
	}

	emit imageUrlCleared(imageIndex);
}
