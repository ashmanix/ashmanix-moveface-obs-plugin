#ifndef MOVEFACEIMAGESOURCE_H
#define MOVEFACEIMAGESOURCE_H

#include <obs-module.h>
#include <graphics/image-file.h>
#include <util/threading.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <sys/stat.h>
#include <mutex>

#include <QImage>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>

// Declare the symbol as 'extern' so it can be defined in moveface-image-source.cpp
extern struct obs_source_info moveface_image_source_info;

const int DEFAULT_TEXTURE_HEIGHT = 400;
const int DEFAULT_TEXTURE_WIDTH = 400;

static const QString NO_POSE_FILE_NAME = "NoPoseImage.png";

struct moveface_image_source {
	obs_source_t *source;

	gs_texture_t *current_texture;

	// texture dimensions
	int texture_width;
	int texture_height;

	bool persistent;
	bool is_slide;
	// bool linear_alpha;
	time_t file_timestamp;
	float update_time_elapsed;
	uint64_t last_time;
	bool active;
	volatile bool file_decoded;
	volatile bool texture_loaded;
};

void image_source_update_texture(void *data, QImage *new_image);

#endif // MOVEFACEIMAGESOURCE_H
