#ifndef BLENDSHAPERULE_H
#define BLENDSHAPERULE_H

#include "blendshape-key.h"
#include "../../utils/utils.h"
// Forward declarations
class Blendshape;

enum class ComparisonType { EQ, LT, LTEQ, GT, GTEQ, COUNT };

class BlendshapeRule {
public:
	explicit BlendshapeRule(QString id = "", BlendshapeKey key = BlendshapeKey::BROWOUTERUP_L,
				ComparisonType cType = ComparisonType::EQ, double value = 0.0);

	QString getID() const;
	BlendshapeKey getKey() const;
	ComparisonType getComparisonType() const;
	double getCompareValue() const;

	void setID(QString newId);
	void setKey(BlendshapeKey newKey);
	void setComparisonType(ComparisonType newType);
	void setCompareValue(double newValue);

	bool evaluate(const Blendshape &blendShape) const;
	static QString comparisonTypeToString(ComparisonType key);

private:
	QString m_id = "";
	BlendshapeKey m_key = BlendshapeKey::BROWOUTERUP_L;
	ComparisonType m_compareType = ComparisonType::EQ;
	double m_compareValue = 0.0;
};

#endif // BLENDSHAPERULE_H
