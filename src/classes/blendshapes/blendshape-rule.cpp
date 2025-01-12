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

QString BlendshapeRule::comparisonTypeToString(ComparisonType key)
{
	// Define a static QHash to ensure it's initialized only once
	static const QHash<ComparisonType, QString> comparisonTypeMap = {{ComparisonType::EQ, "EQ"},
									 {ComparisonType::LT, "LT"},
									 {ComparisonType::LTEQ, "LTEQ"},
									 {ComparisonType::GT, "GT"},
									 {ComparisonType::GTEQ, "GTEQ"}};

	// Attempt to find the key in the map
	if (auto it = comparisonTypeMap.find(key); it != comparisonTypeMap.end()) {
		return it.value();
	}

	// Return an empty QString if the key is not found
	return QString();
}
