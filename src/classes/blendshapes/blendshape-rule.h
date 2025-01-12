#ifndef BLENDSHAPERULE_H
#define BLENDSHAPERULE_H

#include "blendshape-key.h"

// Forward declarations
class Blendshape;

enum class ComparisonType { EQ, LT, LTEQ, GT, GTEQ, COUNT };

class BlendshapeRule {
public:
	BlendshapeKey key;
	ComparisonType compareType;
	double compareValue;

	bool evaluate(const Blendshape &blendShape) const;
	static QString comparisonTypeToString(ComparisonType key);
};

#endif // BLENDSHAPERULE_H
