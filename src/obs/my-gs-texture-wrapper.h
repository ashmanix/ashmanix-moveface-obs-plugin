#pragma once

#include <graphics/graphics.h>
#include <QMetaType>

// This wrapper only exists to get round the issue with Qt not being
// able to get the size of a gs_texture_t because it's full details
// are not exposed by OBS. It hides the opaque gs_texture pointer.
class MyGSTextureWrapper {
public:
	explicit MyGSTextureWrapper(gs_texture_t *tex = nullptr) : texture(tex) {}

	gs_texture_t *get() const { return texture; }
	void set(gs_texture_t *tex) { texture = tex; }

private:
	gs_texture_t *texture;
};

// Register our type with Qt.
Q_DECLARE_METATYPE(MyGSTextureWrapper)
