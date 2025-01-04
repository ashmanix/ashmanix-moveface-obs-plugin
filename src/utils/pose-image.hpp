// pose-image.hpp
#ifndef POSE_IMAGE_HPP
#define POSE_IMAGE_HPP

#include <QString>

// Define the PoseImage enum and related functions here
enum class PoseImage {
	BODY,
	EYESOPEN,
	EYESHALFOPEN,
	EYESCLOSED,
	MOUTHCLOSED,
	MOUTHOPEN,
	MOUTHSMILE,
	TONGUEOUT,
	COUNT,
};

// Optionally, move blendShapeKeyToString and blendShapeKeyFromString here if they are related

#endif // POSE_IMAGE_HPP
