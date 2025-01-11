#ifndef POSEIMAGE_H
#define POSEIMAGE_H

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

inline QString poseImageToString(PoseImage poseImage)
{
	switch (poseImage) {
	case PoseImage::BODY:
		return "BODY";
	case PoseImage::EYESOPEN:
		return "EYESOPEN";
	case PoseImage::EYESHALFOPEN:
		return "EYESHALFOPEN";
	case PoseImage::EYESCLOSED:
		return "EYESCLOSED";
	case PoseImage::MOUTHCLOSED:
		return "MOUTHCLOSED";
	case PoseImage::MOUTHOPEN:
		return "MOUTHOPEN";
	case PoseImage::MOUTHSMILE:
		return "MOUTHSMILE";
	case PoseImage::TONGUEOUT:
		return "TONGUEOUT";
	case PoseImage::COUNT:
		return "COUNT";
	// Handle additional cases
	default:
		return "UNKNOWN_POSE_IMAGE";
	}
}

#endif // POSEIMAGE_H
