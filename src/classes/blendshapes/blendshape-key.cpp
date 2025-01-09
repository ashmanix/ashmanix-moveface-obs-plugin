#include "blendshape-key.h"

QString blendshapeKeyToString(BlendshapeKey key)
{
	switch (key) {
	case BlendshapeKey::BROWOUTERUP_L:
		return "browOuterUp_L";
	case BlendshapeKey::BROWINNERUP_L:
		return "browInnerUp_L";
	case BlendshapeKey::BROWDOWN_L:
		return "browDown_L";
	case BlendshapeKey::EYEBLINK_L:
		return "eyeBlink_L";
	case BlendshapeKey::EYESQUINT_L:
		return "eyeSquint_L";
	case BlendshapeKey::EYEWIDE_L:
		return "eyeWide_L";
	case BlendshapeKey::EYELOOKUP_L:
		return "eyeLookUp_L";
	case BlendshapeKey::EYELOOKOUT_L:
		return "eyeLookOut_L";
	case BlendshapeKey::EYELOOKIN_L:
		return "eyeLookIn_L";
	case BlendshapeKey::EYELOOKDOWN_L:
		return "eyeLookDown_L";
	case BlendshapeKey::NOSESNEER_L:
		return "noseSneer_L";
	case BlendshapeKey::MOUTHUPPERUP_L:
		return "mouthUpperUp_L";
	case BlendshapeKey::MOUTHSMILE_L:
		return "mouthSmile_L";
	case BlendshapeKey::MOUTHLEFT:
		return "mouthLeft";
	case BlendshapeKey::MOUTHFROWN_L:
		return "mouthFrown_L";
	case BlendshapeKey::MOUTHLOWERDOWN_L:
		return "mouthLowerDown_L";
	case BlendshapeKey::JAWLEFT:
		return "jawLeft";
	case BlendshapeKey::CHEEKPUFF:
		return "cheekPuff";
	case BlendshapeKey::MOUTHSHRUGUPPER:
		return "mouthShrugUpper";
	case BlendshapeKey::MOUTHFUNNEL:
		return "mouthFunnel";
	case BlendshapeKey::MOUTHROLLLOWER:
		return "mouthRollLower";
	case BlendshapeKey::JAWOPEN:
		return "jawOpen";
	case BlendshapeKey::TONGUEOUT:
		return "tongueOut";
	case BlendshapeKey::MOUTHPUCKER:
		return "mouthPucker";
	case BlendshapeKey::MOUTHROLLUPPER:
		return "mouthRollUpper";
	case BlendshapeKey::JAWRIGHT:
		return "jawRight";
	case BlendshapeKey::MOUTHLOWERDOWN_R:
		return "mouthLowerDown_R";
	case BlendshapeKey::MOUTHFROWN_R:
		return "mouthFrown_R";
	case BlendshapeKey::MOUTHRIGHT:
		return "mouthRight";
	case BlendshapeKey::MOUTHSMILE_R:
		return "mouthSmile_R";
	case BlendshapeKey::MOUTHUPPERUP_R:
		return "mouthUpperUp_R";
	case BlendshapeKey::NOSESNEER_R:
		return "noseSneer_R";
	case BlendshapeKey::EYELOOKDOWN_R:
		return "eyeLookDown_R";
	case BlendshapeKey::EYELOOKIN_R:
		return "eyeLookIn_R";
	case BlendshapeKey::EYELOOKOUT_R:
		return "eyeLookOut_R";
	case BlendshapeKey::EYELOOKUP_R:
		return "eyeLookUp_R";
	case BlendshapeKey::EYEWIDE_R:
		return "eyeWide_R";
	case BlendshapeKey::EYESQUINT_R:
		return "eyeSquint_R";
	case BlendshapeKey::EYEBLINK_R:
		return "eyeBlink_R";
	case BlendshapeKey::BROWDOWN_R:
		return "browDown_R";
	case BlendshapeKey::BROWINNERUP_R:
		return "browInnerUp_R";
	case BlendshapeKey::BROWOUTERUP_R:
		return "browOuterUp_R";
	case BlendshapeKey::HEADLEFT:
		return "headLeft";
	case BlendshapeKey::HEADRIGHT:
		return "headRight";
	case BlendshapeKey::HEADUP:
		return "headUp";
	case BlendshapeKey::HEADDOWN:
		return "headDown";
	case BlendshapeKey::HEADROLLLEFT:
		return "headRollLeft";
	case BlendshapeKey::HEADROLLRIGHT:
		return "headRollRight";
	case BlendshapeKey::EYEBLINKLEFT:
		return "EyeBlinkLeft";
	case BlendshapeKey::EYEBLINKRIGHT:
		return "EyeBlinkRight";
	default:
		return QString();
	}
}

BlendshapeKey blendshapeKeyFromString(const QString &str)
{
	if (str == "browOuterUp_L")
		return BlendshapeKey::BROWOUTERUP_L;
	if (str == "browInnerUp_L")
		return BlendshapeKey::BROWINNERUP_L;
	if (str == "browDown_L")
		return BlendshapeKey::BROWDOWN_L;
	if (str == "eyeBlink_L")
		return BlendshapeKey::EYEBLINK_L;
	if (str == "eyeSquint_L")
		return BlendshapeKey::EYESQUINT_L;
	if (str == "eyeWide_L")
		return BlendshapeKey::EYEWIDE_L;
	if (str == "eyeLookUp_L")
		return BlendshapeKey::EYELOOKUP_L;
	if (str == "eyeLookOut_L")
		return BlendshapeKey::EYELOOKOUT_L;
	if (str == "eyeLookIn_L")
		return BlendshapeKey::EYELOOKIN_L;
	if (str == "eyeLookDown_L")
		return BlendshapeKey::EYELOOKDOWN_L;
	if (str == "noseSneer_L")
		return BlendshapeKey::NOSESNEER_L;
	if (str == "mouthUpperUp_L")
		return BlendshapeKey::MOUTHUPPERUP_L;
	if (str == "mouthSmile_L")
		return BlendshapeKey::MOUTHSMILE_L;
	if (str == "mouthLeft")
		return BlendshapeKey::MOUTHLEFT;
	if (str == "mouthFrown_L")
		return BlendshapeKey::MOUTHFROWN_L;
	if (str == "mouthLowerDown_L")
		return BlendshapeKey::MOUTHLOWERDOWN_L;
	if (str == "jawLeft")
		return BlendshapeKey::JAWLEFT;
	if (str == "cheekPuff")
		return BlendshapeKey::CHEEKPUFF;
	if (str == "mouthShrugUpper")
		return BlendshapeKey::MOUTHSHRUGUPPER;
	if (str == "mouthFunnel")
		return BlendshapeKey::MOUTHFUNNEL;
	if (str == "mouthRollLower")
		return BlendshapeKey::MOUTHROLLLOWER;
	if (str == "jawOpen")
		return BlendshapeKey::JAWOPEN;
	if (str == "tongueOut")
		return BlendshapeKey::TONGUEOUT;
	if (str == "mouthPucker")
		return BlendshapeKey::MOUTHPUCKER;
	if (str == "mouthRollUpper")
		return BlendshapeKey::MOUTHROLLUPPER;
	if (str == "jawRight")
		return BlendshapeKey::JAWRIGHT;
	if (str == "mouthLowerDown_R")
		return BlendshapeKey::MOUTHLOWERDOWN_R;
	if (str == "mouthFrown_R")
		return BlendshapeKey::MOUTHFROWN_R;
	if (str == "mouthRight")
		return BlendshapeKey::MOUTHRIGHT;
	if (str == "mouthSmile_R")
		return BlendshapeKey::MOUTHSMILE_R;
	if (str == "mouthUpperUp_R")
		return BlendshapeKey::MOUTHUPPERUP_R;
	if (str == "noseSneer_R")
		return BlendshapeKey::NOSESNEER_R;
	if (str == "eyeLookDown_R")
		return BlendshapeKey::EYELOOKDOWN_R;
	if (str == "eyeLookIn_R")
		return BlendshapeKey::EYELOOKIN_R;
	if (str == "eyeLookOut_R")
		return BlendshapeKey::EYELOOKOUT_R;
	if (str == "eyeLookUp_R")
		return BlendshapeKey::EYELOOKUP_R;
	if (str == "eyeWide_R")
		return BlendshapeKey::EYEWIDE_R;
	if (str == "eyeSquint_R")
		return BlendshapeKey::EYESQUINT_R;
	if (str == "eyeBlink_R")
		return BlendshapeKey::EYEBLINK_R;
	if (str == "browDown_R")
		return BlendshapeKey::BROWDOWN_R;
	if (str == "browInnerUp_R")
		return BlendshapeKey::BROWINNERUP_R;
	if (str == "browOuterUp_R")
		return BlendshapeKey::BROWOUTERUP_R;
	if (str == "headLeft")
		return BlendshapeKey::HEADLEFT;
	if (str == "headRight")
		return BlendshapeKey::HEADRIGHT;
	if (str == "headUp")
		return BlendshapeKey::HEADUP;
	if (str == "headDown")
		return BlendshapeKey::HEADDOWN;
	if (str == "headRollLeft")
		return BlendshapeKey::HEADROLLLEFT;
	if (str == "headRollRight")
		return BlendshapeKey::HEADROLLRIGHT;
	if (str == "EyeBlinkLeft")
		return BlendshapeKey::EYEBLINKLEFT;
	if (str == "EyeBlinkRight")
		return BlendshapeKey::EYEBLINKRIGHT;

	return BlendshapeKey::UNKNOWN;
}
