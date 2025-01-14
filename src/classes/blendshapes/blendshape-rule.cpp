#include "blendshape-rule.h"
#include "blendshape.h"

BlendshapeRule::BlendshapeRule(QString id, BlendshapeKey key, ComparisonType cType, double value)
	: m_id(id),
	  m_key(key),
	  m_compareType(cType),
	  m_compareValue(value)
{
	if (m_id.isEmpty()) {
		// Get a unique ID for blendshape rule
		m_id = generateUniqueID();
	}
}

QString BlendshapeRule::getID() const
{
	return m_id;
};

BlendshapeKey BlendshapeRule::getKey() const
{
	return m_key;
};

ComparisonType BlendshapeRule::getComparisonType() const
{
	return m_compareType;
};

double BlendshapeRule::getCompareValue() const
{
	return m_compareValue;
};

void BlendshapeRule::setID(QString newId)
{
	m_id = newId;
};

void BlendshapeRule::setKey(BlendshapeKey newKey)
{
	m_key = newKey;
};

void BlendshapeRule::setComparisonType(ComparisonType newType)
{
	m_compareType = newType;
};

void BlendshapeRule::setCompareValue(double newValue)
{
	m_compareValue = newValue;
};

bool BlendshapeRule::evaluate(const Blendshape &blendShape) const
{
	if (blendShape.m_key == BlendshapeKey::UNKNOWN)
		return false;

	double valueToCompare = blendShape.m_value;

	switch (m_compareType) {
	case ComparisonType::EQ:
		return valueToCompare == m_compareValue;
	case ComparisonType::LT:
		return valueToCompare < m_compareValue;
	case ComparisonType::LTEQ:
		return valueToCompare <= m_compareValue;
	case ComparisonType::GT:
		return valueToCompare > m_compareValue;
	case ComparisonType::GTEQ:
		return valueToCompare >= m_compareValue;

	default:
		return false;
		break;
	}
}

QString BlendshapeRule::comparisonTypeToString(ComparisonType key)
{
	// Define a static QHash to ensure it's initialized only once
	static const QHash<ComparisonType, QString> comparisonTypeMap = {{ComparisonType::EQ, "=="},
									 {ComparisonType::LT, "<"},
									 {ComparisonType::LTEQ, "<="},
									 {ComparisonType::GT, ">"},
									 {ComparisonType::GTEQ, ">="}};

	// Attempt to find the key in the map
	if (auto it = comparisonTypeMap.find(key); it != comparisonTypeMap.end()) {
		return it.value();
	}

	// Return an empty QString if the key is not found
	return QString();
}
