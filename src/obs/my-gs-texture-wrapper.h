#pragma once

// This wrapper only exists to get round the issue with Qt not being
// able to get the size of a gs_texture_t because it's full details
// are not exposed by OBS
#include <graphics/graphics.h>

// Our wrapper hides the opaque gs_texture pointer.
class MyGSTextureWrapper {
public:
	MyGSTextureWrapper(gs_texture_t *tex = nullptr) : texture(tex) {}

	// You can add methods to update or retrieve the texture as needed.
	gs_texture_t *get() const { return texture; }
	void set(gs_texture_t *tex) { texture = tex; }

private:
	gs_texture_t *texture;
};

// Register our type with Qt.
#include <QMetaType>
Q_DECLARE_METATYPE(MyGSTextureWrapper)
