#include "moveface-image-source.h"
#include "moveface-source-private.h"
#include "../plugin-support.h"

void image_source_update_texture(void *data, gs_texture_t *new_texture, int width, int height)
{
	obs_log(LOG_INFO, "Updating texture!");
	auto context = (struct moveface_image_source *)data;

	obs_enter_graphics();

	if (context->current_texture) {
		gs_texture_destroy(context->current_texture);
		context->current_texture = nullptr;
	}

	if (new_texture) {
		// Set the new texture and update dimensions
		context->current_texture = new_texture;
		context->texture_width = width;
		context->texture_height = height;
		os_atomic_set_bool(&context->texture_loaded, true);
	} else {
		os_atomic_set_bool(&context->texture_loaded, false);
		obs_log(LOG_INFO, "Failed to load texture!");
	}

	obs_leave_graphics();
}

static const char *image_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("CustomImageSourceName");
}

static void image_source_load_texture(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;
	if (os_atomic_load_bool(&context->texture_loaded))
		return;

	if (context->current_texture) {
		context->texture_width = gs_texture_get_width(context->current_texture);
		context->texture_height = gs_texture_get_height(context->current_texture);

		context->update_time_elapsed = 0;
		os_atomic_set_bool(&context->texture_loaded, true);
	} else {
		obs_log(LOG_WARNING, "No texture provided.");
	}
}

static void image_source_unload(void *data)
{
	struct moveface_image_source *context = (moveface_image_source *)data;
	os_atomic_set_bool(&context->file_decoded, false);
	os_atomic_set_bool(&context->texture_loaded, false);

	obs_enter_graphics();
	gs_texture_destroy(context->current_texture);
	// gs_image_file4_free(&context->if4);
	obs_leave_graphics();
}

static void image_source_load(struct moveface_image_source *context)
{
	image_source_unload(context);

	if (context->current_texture) {
		// image_source_preload_image(context);
		image_source_load_texture(context);
	}
}

static void image_source_update(void *data, obs_data_t *settings)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;
	// const char *file = obs_data_get_string(settings, "file");
	const bool unload = obs_data_get_bool(settings, "unload");
	// const bool linear_alpha = obs_data_get_bool(settings, "linear_alpha");
	const bool is_slide = obs_data_get_bool(settings, "is_slide");

	context->persistent = !unload;
	// context->linear_alpha = linear_alpha;
	context->is_slide = is_slide;

	if (is_slide)
		return;

	/* Load the image if the source is persistent or showing */
	if (context->persistent || obs_source_showing(context->source))
		image_source_load((struct moveface_image_source *)data);
	else
		image_source_unload(data);
}

static void image_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, "unload", false);
	// obs_data_set_default_bool(settings, "linear_alpha", false);
}

static void image_source_show(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;

	if (!context->persistent && !context->is_slide)
		image_source_load(context);
}

static void image_source_hide(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;

	if (!context->persistent && !context->is_slide)
		image_source_unload(context);
}

static void image_source_activate(void *data)
{
	UNUSED_PARAMETER(data);
	// struct moveface_image_source *context = (struct moveface_image_source *)data;
	// context->restart_gif = true;
}

static void *image_source_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);

	struct moveface_image_source *context =
		(struct moveface_image_source *)bzalloc(sizeof(struct moveface_image_source));
	context->source = source;
	context->texture_width = 0;
	context->texture_height = 0;
	context->source = source;
	context->current_texture = nullptr;
	os_atomic_set_bool(&context->texture_loaded, false);

	// image_source_update(context, settings);
	{
		QMutexLocker locker(&g_sourceDataMutex);
		g_sourceDataMap.insert(source, context);
	}

	return context;
}

static void image_source_destroy(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;

	{
		QMutexLocker locker(&g_sourceDataMutex);
		g_sourceDataMap.remove(context->source);
	}

	image_source_unload(context);

	bfree(context);
}

static uint32_t image_source_getwidth(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;
	return context->texture_height;
}

static uint32_t image_source_getheight(void *data)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;
	return context->texture_width;
}

static void image_source_render(void *data, gs_effect_t *effect)
{
	struct moveface_image_source *context = (struct moveface_image_source *)data;
	if (!os_atomic_load_bool(&context->texture_loaded))
		return;

	gs_texture_t *texture = context->current_texture;
	if (!texture)
		return;

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);

	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);

	gs_eparam_t *const param = gs_effect_get_param_by_name(effect, "image");
	gs_effect_set_texture_srgb(param, texture);

	gs_draw_sprite(texture, 0, context->texture_width, context->texture_height);

	gs_blend_state_pop();

	gs_enable_framebuffer_srgb(previous);
}

static obs_properties_t *image_source_properties(void *data)
{
	UNUSED_PARAMETER(data);

	obs_properties_t *props = obs_properties_create();

	obs_properties_add_bool(props, "unload", obs_module_text("UnloadWhenNotShowing"));
	obs_properties_add_bool(props, "linear_alpha", obs_module_text("LinearAlpha"));

	return props;
}

struct obs_source_info moveface_image_source_info = {
	.id = "moveface_image_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB,
	.get_name = image_source_get_name,
	.create = image_source_create,
	.destroy = image_source_destroy,
	.update = image_source_update,
	.get_defaults = image_source_defaults,
	.show = image_source_show,
	.hide = image_source_hide,
	.get_width = image_source_getwidth,
	.get_height = image_source_getheight,
	.video_render = image_source_render,
	// .video_tick = image_source_tick,
	.get_properties = image_source_properties,
	.icon_type = OBS_ICON_TYPE_IMAGE,
	.activate = image_source_activate,
	// .video_get_color_space = image_source_get_color_space,
};
