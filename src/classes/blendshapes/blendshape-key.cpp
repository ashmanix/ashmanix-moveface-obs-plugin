#include "blendshape-key.h"

QString blendshapeKeyToString(BlendshapeKey key)
{
	// Define a static QHash to ensure it's initialized only once
	static const QHash<BlendshapeKey, QString> blendshapeMap = {
		{BROWOUTERUP_L, "browOuterUp_L"},
		{BROWINNERUP_L, "browInnerUp_L"},
		{BROWDOWN_L, "browDown_L"},
		{EYEBLINK_L, "eyeBlink_L"},
		{EYESQUINT_L, "eyeSquint_L"},
		{EYEWIDE_L, "eyeWide_L"},
		{EYELOOKUP_L, "eyeLookUp_L"},
		{EYELOOKOUT_L, "eyeLookOut_L"},
		{EYELOOKIN_L, "eyeLookIn_L"},
		{EYELOOKDOWN_L, "eyeLookDown_L"},
		{NOSESNEER_L, "noseSneer_L"},
		{MOUTHUPPERUP_L, "mouthUpperUp_L"},
		{MOUTHSMILE_L, "mouthSmile_L"},
		{MOUTHLEFT, "mouthLeft"},
		{MOUTHFROWN_L, "mouthFrown_L"},
		{MOUTHLOWERDOWN_L, "mouthLowerDown_L"},
		{JAWLEFT, "jawLeft"},
		{CHEEKPUFF, "cheekPuff"},
		{MOUTHSHRUGUPPER, "mouthShrugUpper"},
		{MOUTHFUNNEL, "mouthFunnel"},
		{MOUTHROLLLOWER, "mouthRollLower"},
		{JAWOPEN, "jawOpen"},
		{TONGUEOUT, "tongueOut"},
		{MOUTHPUCKER, "mouthPucker"},
		{MOUTHROLLUPPER, "mouthRollUpper"},
		{JAWRIGHT, "jawRight"},
		{MOUTHLOWERDOWN_R, "mouthLowerDown_R"},
		{MOUTHFROWN_R, "mouthFrown_R"},
		{MOUTHRIGHT, "mouthRight"},
		{MOUTHSMILE_R, "mouthSmile_R"},
		{MOUTHUPPERUP_R, "mouthUpperUp_R"},
		{NOSESNEER_R, "noseSneer_R"},
		{EYELOOKDOWN_R, "eyeLookDown_R"},
		{EYELOOKIN_R, "eyeLookIn_R"},
		{EYELOOKOUT_R, "eyeLookOut_R"},
		{EYELOOKUP_R, "eyeLookUp_R"},
		{EYEWIDE_R, "eyeWide_R"},
		{EYESQUINT_R, "eyeSquint_R"},
		{EYEBLINK_R, "eyeBlink_R"},
		{BROWDOWN_R, "browDown_R"},
		{BROWINNERUP_R, "browInnerUp_R"},
		{BROWOUTERUP_R, "browOuterUp_R"},
		{HEADLEFT, "headLeft"},
		{HEADRIGHT, "headRight"},
		{HEADUP, "headUp"},
		{HEADDOWN, "headDown"},
		{HEADROLLLEFT, "headRollLeft"},
		{HEADROLLRIGHT, "headRollRight"},
		{EYEBLINKLEFT, "EyeBlinkLeft"},
		{EYEBLINKRIGHT, "EyeBlinkRight"}
		// Add all other mappings here
	};

	// Attempt to find the key in the map
	auto it = blendshapeMap.find(key);
	if (it != blendshapeMap.end()) {
		return it.value();
	}

	// Return an empty QString if the key is not found
	return QString();
}

BlendshapeKey blendshapeKeyFromString(const QString &str)
{
	static const std::unordered_map<QString, BlendshapeKey> mapping = {
		{"browOuterUp_L", BROWOUTERUP_L},
		{"browInnerUp_L", BROWINNERUP_L},
		{"browDown_L", BROWDOWN_L},
		{"eyeBlink_L", EYEBLINK_L},
		{"eyeSquint_L", EYESQUINT_L},
		{"eyeWide_L", EYEWIDE_L},
		{"eyeLookUp_L", EYELOOKUP_L},
		{"eyeLookOut_L", EYELOOKOUT_L},
		{"eyeLookIn_L", EYELOOKIN_L},
		{"eyeLookDown_L", EYELOOKDOWN_L},
		{"noseSneer_L", NOSESNEER_L},
		{"mouthUpperUp_L", MOUTHUPPERUP_L},
		{"mouthSmile_L", MOUTHSMILE_L},
		{"mouthLeft", MOUTHLEFT},
		{"mouthFrown_L", MOUTHFROWN_L},
		{"mouthLowerDown_L", MOUTHLOWERDOWN_L},
		{"jawLeft", JAWLEFT},
		{"cheekPuff", CHEEKPUFF},
		{"mouthShrugUpper", MOUTHSHRUGUPPER},
		{"mouthFunnel", MOUTHFUNNEL},
		{"mouthRollLower", MOUTHROLLLOWER},
		{"jawOpen", JAWOPEN},
		{"tongueOut", TONGUEOUT},
		{"mouthPucker", MOUTHPUCKER},
		{"mouthRollUpper", MOUTHROLLUPPER},
		{"jawRight", JAWRIGHT},
		{"mouthLowerDown_R", MOUTHLOWERDOWN_R},
		{"mouthFrown_R", MOUTHFROWN_R},
		{"mouthRight", MOUTHRIGHT},
		{"mouthSmile_R", MOUTHSMILE_R},
		{"mouthUpperUp_R", MOUTHUPPERUP_R},
		{"noseSneer_R", NOSESNEER_R},
		{"eyeLookDown_R", EYELOOKDOWN_R},
		{"eyeLookIn_R", EYELOOKIN_R},
		{"eyeLookOut_R", EYELOOKOUT_R},
		{"eyeLookUp_R", EYELOOKUP_R},
		{"eyeWide_R", EYEWIDE_R},
		{"eyeSquint_R", EYESQUINT_R},
		{"eyeBlink_R", EYEBLINK_R},
		{"browDown_R", BROWDOWN_R},
		{"browInnerUp_R", BROWINNERUP_R},
		{"browOuterUp_R", BROWOUTERUP_R},
		{"headLeft", HEADLEFT},
		{"headRight", HEADRIGHT},
		{"headUp", HEADUP},
		{"headDown", HEADDOWN},
		{"headRollLeft", HEADROLLLEFT},
		{"headRollRight", HEADROLLRIGHT},
		{"EyeBlinkLeft", EYEBLINKLEFT},
		{"EyeBlinkRight", EYEBLINKRIGHT},
	};

	auto it = mapping.find(str);
	if (it != mapping.end())
		return it->second;
	return UNKNOWN;
}
