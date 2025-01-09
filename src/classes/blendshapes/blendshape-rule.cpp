#include "blendshape-rule.h"
#include "blendshape.h"

bool BlendshapeRule::evaluate(const Blendshape &blendShape) const
{
	if (blendShape.key == BlendshapeKey::UNKNOWN)
		return false;

	double valueToCompare = blendShape.value;

	switch (compareType) {
	case ComparisonType::EQ:
		return valueToCompare == compareValue;
	case ComparisonType::LT:
		return valueToCompare < compareValue;
	case ComparisonType::LTEQ:
		return valueToCompare <= compareValue;
	case ComparisonType::GT:
		return valueToCompare > compareValue;
	case ComparisonType::GTEQ:
		return valueToCompare >= compareValue;

	default:
		return false;
		break;
	}
}
