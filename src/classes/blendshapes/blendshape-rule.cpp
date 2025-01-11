#include "blendshape-rule.h"
#include "blendshape.h"

bool BlendshapeRule::evaluate(const Blendshape &blendShape) const
{
	if (blendShape.key == BlendshapeKey::UNKNOWN)
		return false;

	double valueToCompare = blendShape.value;

	switch (compareType) {
	case EQ:
		return valueToCompare == compareValue;
	case LT:
		return valueToCompare < compareValue;
	case LTEQ:
		return valueToCompare <= compareValue;
	case GT:
		return valueToCompare > compareValue;
	case GTEQ:
		return valueToCompare >= compareValue;

	default:
		return false;
		break;
	}
}
