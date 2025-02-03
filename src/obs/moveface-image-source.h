#ifndef MOVEFACEIMAGESOURCE_H
#define MOVEFACEIMAGESOURCE_H

#include <obs-module.h>
#include <graphics/image-file.h>
#include <util/threading.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <sys/stat.h>
#include <mutex>

#include <QMap>
#include <QMutex>
#include <QMutexLocker>

// Declare the symbol as 'extern' so it can be defined in moveface-image-source.cpp
extern struct obs_source_info moveface_image_source_info;

struct moveface_image_source {
	obs_source_t *source;
	const char *test = "yo";

	gs_texture_t *current_texture;

	// texture dimensions
	int texture_width, texture_height;

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

void image_source_update_texture(void *data, gs_texture_t *new_texture, int width, int height);

#endif // MOVEFACEIMAGESOURCE_H
