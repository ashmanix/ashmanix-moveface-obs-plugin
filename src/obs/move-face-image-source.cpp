#include <obs.h>
#include <graphics/image-file.h>

struct MoveFaceImageSource {
	obs_source_t *source;
	gs_texture_t *current_texture; // The texture that the image source displays
	int width;
	int height;
};

void *moveFaceImageSourceCreate(obs_data_t *settings, obs_source_t *source)
{
	struct MoveFaceImageSource *data = (struct MoveFaceImageSource *)malloc(sizeof(struct MoveFaceImageSource));
	data->source = source;
	data->current_texture = nullptr; // No texture initially
	data->width = 1920;              // Default width
	data->height = 1080;             // Default height
	return data;
}

void moveFaceImageSourceUpdate(void *data, gs_texture_t *new_texture)
{
	struct MoveFaceImageSource *image_data = (struct MoveFaceImageSource *)data;
	if (image_data->current_texture) {
		gs_texture_destroy(image_data->current_texture);
	}
	image_data->current_texture = new_texture;
}

void moveFaceImageSourceRender(void *data)
{
	struct MoveFaceImageSource *image_data = (struct MoveFaceImageSource *)data;
	if (image_data->current_texture) {
		// Render the texture onto the source
		gs_primitive_texture(image_data->current_texture, 0.0f, 0.0f, image_data->width, image_data->height,
				     0.0f);
	}
}

void moveFaceImageSourceDestroy(void *data)
{
	struct MoveFaceImageSource *image_data = (struct MoveFaceImageSource *)data;
	if (image_data->current_texture) {
		gs_texture_destroy(image_data->current_texture);
	}
	free(image_data);
}

static struct obs_source_info custom_image_source_info = {
	.id = "custom_image_source",   // Unique ID for your custom source
	.type = OBS_SOURCE_TYPE_INPUT, // Input source type
	.create = moveFaceImageSourceCreate,
	.update = nullptr, // No update function needed for this custom source
	.video_render = moveFaceImageSourceRender,
	.destroy = moveFaceImageSourceDestroy,
	.get_width = [](void *data) { return ((struct MoveFaceImageSource *)data)->width; },
	.get_height = [](void *data) { return ((struct MoveFaceImageSource *)data)->height; },
};
