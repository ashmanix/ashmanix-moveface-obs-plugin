#include "blendshape-key.h"

QString blendshapeKeyToString(BlendshapeKey key)
{
	// Define a static QHash to ensure it's initialized only once
	static const QHash<BlendshapeKey, QString> blendshapeMap = {
		{BlendshapeKey::BROWOUTERUP_L, "browOuterUp_L"},
		{BlendshapeKey::BROWINNERUP_L, "browInnerUp_L"},
		{BlendshapeKey::BROWDOWN_L, "browDown_L"},
		{BlendshapeKey::EYEBLINK_L, "eyeBlink_L"},
		{BlendshapeKey::EYESQUINT_L, "eyeSquint_L"},
		{BlendshapeKey::EYEWIDE_L, "eyeWide_L"},
		{BlendshapeKey::EYELOOKUP_L, "eyeLookUp_L"},
		{BlendshapeKey::EYELOOKOUT_L, "eyeLookOut_L"},
		{BlendshapeKey::EYELOOKIN_L, "eyeLookIn_L"},
		{BlendshapeKey::EYELOOKDOWN_L, "eyeLookDown_L"},
		{BlendshapeKey::NOSESNEER_L, "noseSneer_L"},
		{BlendshapeKey::MOUTHUPPERUP_L, "mouthUpperUp_L"},
		{BlendshapeKey::MOUTHSMILE_L, "mouthSmile_L"},
		{BlendshapeKey::MOUTHLEFT, "mouthLeft"},
		{BlendshapeKey::MOUTHFROWN_L, "mouthFrown_L"},
		{BlendshapeKey::MOUTHLOWERDOWN_L, "mouthLowerDown_L"},
		{BlendshapeKey::JAWLEFT, "jawLeft"},
		{BlendshapeKey::CHEEKPUFF, "cheekPuff"},
		{BlendshapeKey::MOUTHSHRUGUPPER, "mouthShrugUpper"},
		{BlendshapeKey::MOUTHFUNNEL, "mouthFunnel"},
		{BlendshapeKey::MOUTHROLLLOWER, "mouthRollLower"},
		{BlendshapeKey::JAWOPEN, "jawOpen"},
		{BlendshapeKey::TONGUEOUT, "tongueOut"},
		{BlendshapeKey::MOUTHPUCKER, "mouthPucker"},
		{BlendshapeKey::MOUTHROLLUPPER, "mouthRollUpper"},
		{BlendshapeKey::JAWRIGHT, "jawRight"},
		{BlendshapeKey::MOUTHLOWERDOWN_R, "mouthLowerDown_R"},
		{BlendshapeKey::MOUTHFROWN_R, "mouthFrown_R"},
		{BlendshapeKey::MOUTHRIGHT, "mouthRight"},
		{BlendshapeKey::MOUTHSMILE_R, "mouthSmile_R"},
		{BlendshapeKey::MOUTHUPPERUP_R, "mouthUpperUp_R"},
		{BlendshapeKey::NOSESNEER_R, "noseSneer_R"},
		{BlendshapeKey::EYELOOKDOWN_R, "eyeLookDown_R"},
		{BlendshapeKey::EYELOOKIN_R, "eyeLookIn_R"},
		{BlendshapeKey::EYELOOKOUT_R, "eyeLookOut_R"},
		{BlendshapeKey::EYELOOKUP_R, "eyeLookUp_R"},
		{BlendshapeKey::EYEWIDE_R, "eyeWide_R"},
		{BlendshapeKey::EYESQUINT_R, "eyeSquint_R"},
		{BlendshapeKey::EYEBLINK_R, "eyeBlink_R"},
		{BlendshapeKey::BROWDOWN_R, "browDown_R"},
		{BlendshapeKey::BROWINNERUP_R, "browInnerUp_R"},
		{BlendshapeKey::BROWOUTERUP_R, "browOuterUp_R"},
		{BlendshapeKey::HEADLEFT, "headLeft"},
		{BlendshapeKey::HEADRIGHT, "headRight"},
		{BlendshapeKey::HEADUP, "headUp"},
		{BlendshapeKey::HEADDOWN, "headDown"},
		{BlendshapeKey::HEADROLLLEFT, "headRollLeft"},
		{BlendshapeKey::HEADROLLRIGHT, "headRollRight"},
		{BlendshapeKey::EYEBLINKLEFT, "EyeBlinkLeft"},
		{BlendshapeKey::EYEBLINKRIGHT, "EyeBlinkRight"}
		// Add all other mappings here
	};

	// Attempt to find the key in the map
	if (auto it = blendshapeMap.find(key); it != blendshapeMap.end()) {
		return it.value();
	}

	// Return an empty QString if the key is not found
	return QString();
}

BlendshapeKey blendshapeKeyFromString(const QString &str)
{
	static const std::unordered_map<QString, BlendshapeKey> mapping = {
		{"browOuterUp_L", BlendshapeKey::BROWOUTERUP_L},
		{"browInnerUp_L", BlendshapeKey::BROWINNERUP_L},
		{"browDown_L", BlendshapeKey::BROWDOWN_L},
		{"eyeBlink_L", BlendshapeKey::EYEBLINK_L},
		{"eyeSquint_L", BlendshapeKey::EYESQUINT_L},
		{"eyeWide_L", BlendshapeKey::EYEWIDE_L},
		{"eyeLookUp_L", BlendshapeKey::EYELOOKUP_L},
		{"eyeLookOut_L", BlendshapeKey::EYELOOKOUT_L},
		{"eyeLookIn_L", BlendshapeKey::EYELOOKIN_L},
		{"eyeLookDown_L", BlendshapeKey::EYELOOKDOWN_L},
		{"noseSneer_L", BlendshapeKey::NOSESNEER_L},
		{"mouthUpperUp_L", BlendshapeKey::MOUTHUPPERUP_L},
		{"mouthSmile_L", BlendshapeKey::MOUTHSMILE_L},
		{"mouthLeft", BlendshapeKey::MOUTHLEFT},
		{"mouthFrown_L", BlendshapeKey::MOUTHFROWN_L},
		{"mouthLowerDown_L", BlendshapeKey::MOUTHLOWERDOWN_L},
		{"jawLeft", BlendshapeKey::JAWLEFT},
		{"cheekPuff", BlendshapeKey::CHEEKPUFF},
		{"mouthShrugUpper", BlendshapeKey::MOUTHSHRUGUPPER},
		{"mouthFunnel", BlendshapeKey::MOUTHFUNNEL},
		{"mouthRollLower", BlendshapeKey::MOUTHROLLLOWER},
		{"jawOpen", BlendshapeKey::JAWOPEN},
		{"tongueOut", BlendshapeKey::TONGUEOUT},
		{"mouthPucker", BlendshapeKey::MOUTHPUCKER},
		{"mouthRollUpper", BlendshapeKey::MOUTHROLLUPPER},
		{"jawRight", BlendshapeKey::JAWRIGHT},
		{"mouthLowerDown_R", BlendshapeKey::MOUTHLOWERDOWN_R},
		{"mouthFrown_R", BlendshapeKey::MOUTHFROWN_R},
		{"mouthRight", BlendshapeKey::MOUTHRIGHT},
		{"mouthSmile_R", BlendshapeKey::MOUTHSMILE_R},
		{"mouthUpperUp_R", BlendshapeKey::MOUTHUPPERUP_R},
		{"noseSneer_R", BlendshapeKey::NOSESNEER_R},
		{"eyeLookDown_R", BlendshapeKey::EYELOOKDOWN_R},
		{"eyeLookIn_R", BlendshapeKey::EYELOOKIN_R},
		{"eyeLookOut_R", BlendshapeKey::EYELOOKOUT_R},
		{"eyeLookUp_R", BlendshapeKey::EYELOOKUP_R},
		{"eyeWide_R", BlendshapeKey::EYEWIDE_R},
		{"eyeSquint_R", BlendshapeKey::EYESQUINT_R},
		{"eyeBlink_R", BlendshapeKey::EYEBLINK_R},
		{"browDown_R", BlendshapeKey::BROWDOWN_R},
		{"browInnerUp_R", BlendshapeKey::BROWINNERUP_R},
		{"browOuterUp_R", BlendshapeKey::BROWOUTERUP_R},
		{"headLeft", BlendshapeKey::HEADLEFT},
		{"headRight", BlendshapeKey::HEADRIGHT},
		{"headUp", BlendshapeKey::HEADUP},
		{"headDown", BlendshapeKey::HEADDOWN},
		{"headRollLeft", BlendshapeKey::HEADROLLLEFT},
		{"headRollRight", BlendshapeKey::HEADROLLRIGHT},
		{"EyeBlinkLeft", BlendshapeKey::EYEBLINKLEFT},
		{"EyeBlinkRight", BlendshapeKey::EYEBLINKRIGHT},
	};

	if (auto it = mapping.find(str); it != mapping.end()) {
		return it->second;
	}
	return BlendshapeKey::UNKNOWN;
}
