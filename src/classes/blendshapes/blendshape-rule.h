#ifndef BLENDSHAPERULE_H
#define BLENDSHAPERULE_H

#include "blendshape-key.h"

// Forward declarations
class Blendshape;

enum class ComparisonType { EQ, LT, LTEQ, GT, GTEQ };

class BlendshapeRule {
public:
	BlendshapeKey key;
	ComparisonType compareType;
	double compareValue;

	bool evaluate(const Blendshape &blendShape) const;
};

#endif // BLENDSHAPERULE_H
