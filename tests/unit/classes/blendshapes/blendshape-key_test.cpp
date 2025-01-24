#include <gtest/gtest.h>
#include "../src/classes/blendshapes/blendshape-key.h"

// Include Qt headers
#include <QString>

// // Test Suite for blendshapeKeyToString
// TEST(BlendshapeKeyToStringTest, ValidKeys) {
//     struct TestCase {
//         BlendshapeKey key;
//         QString expectedString;
//     };

//     // Define all valid key-string pairs
//     TestCase testCases[] = {
//         {BlendshapeKey::BROWOUTERUP_L, "browOuterUp_L"},
//         {BlendshapeKey::BROWINNERUP_L, "browInnerUp_L"},
//         {BlendshapeKey::BROWDOWN_L, "browDown_L"},
//         {BlendshapeKey::EYEBLINK_L, "eyeBlink_L"},
//         {BlendshapeKey::EYESQUINT_L, "eyeSquint_L"},
//         {BlendshapeKey::EYEWIDE_L, "eyeWide_L"},
//         {BlendshapeKey::EYELOOKUP_L, "eyeLookUp_L"},
//         {BlendshapeKey::EYELOOKOUT_L, "eyeLookOut_L"},
//         {BlendshapeKey::EYELOOKIN_L, "eyeLookIn_L"},
//         {BlendshapeKey::EYELOOKDOWN_L, "eyeLookDown_L"},
//         {BlendshapeKey::NOSESNEER_L, "noseSneer_L"},
//         {BlendshapeKey::MOUTHUPPERUP_L, "mouthUpperUp_L"},
//         {BlendshapeKey::MOUTHSMILE_L, "mouthSmile_L"},
//         {BlendshapeKey::MOUTHLEFT, "mouthLeft"},
//         {BlendshapeKey::MOUTHFROWN_L, "mouthFrown_L"},
//         {BlendshapeKey::MOUTHLOWERDOWN_L, "mouthLowerDown_L"},
//         {BlendshapeKey::JAWLEFT, "jawLeft"},
//         {BlendshapeKey::CHEEKPUFF, "cheekPuff"},
//         {BlendshapeKey::MOUTHSHRUGUPPER, "mouthShrugUpper"},
//         {BlendshapeKey::MOUTHFUNNEL, "mouthFunnel"},
//         {BlendshapeKey::MOUTHROLLLOWER, "mouthRollLower"},
//         {BlendshapeKey::JAWOPEN, "jawOpen"},
//         {BlendshapeKey::TONGUEOUT, "tongueOut"},
//         {BlendshapeKey::MOUTHPUCKER, "mouthPucker"},
//         {BlendshapeKey::MOUTHROLLUPPER, "mouthRollUpper"},
//         {BlendshapeKey::JAWRIGHT, "jawRight"},
//         {BlendshapeKey::MOUTHLOWERDOWN_R, "mouthLowerDown_R"},
//         {BlendshapeKey::MOUTHFROWN_R, "mouthFrown_R"},
//         {BlendshapeKey::MOUTHRIGHT, "mouthRight"},
//         {BlendshapeKey::MOUTHSMILE_R, "mouthSmile_R"},
//         {BlendshapeKey::MOUTHUPPERUP_R, "mouthUpperUp_R"},
//         {BlendshapeKey::NOSESNEER_R, "noseSneer_R"},
//         {BlendshapeKey::EYELOOKDOWN_R, "eyeLookDown_R"},
//         {BlendshapeKey::EYELOOKIN_R, "eyeLookIn_R"},
//         {BlendshapeKey::EYELOOKOUT_R, "eyeLookOut_R"},
//         {BlendshapeKey::EYELOOKUP_R, "eyeLookUp_R"},
//         {BlendshapeKey::EYEWIDE_R, "eyeWide_R"},
//         {BlendshapeKey::EYESQUINT_R, "eyeSquint_R"},
//         {BlendshapeKey::EYEBLINK_R, "eyeBlink_R"},
//         {BlendshapeKey::BROWDOWN_R, "browDown_R"},
//         {BlendshapeKey::BROWINNERUP_R, "browInnerUp_R"},
//         {BlendshapeKey::BROWOUTERUP_R, "browOuterUp_R"},
//         {BlendshapeKey::HEADLEFT, "headLeft"},
//         {BlendshapeKey::HEADRIGHT, "headRight"},
//         {BlendshapeKey::HEADUP, "headUp"},
//         {BlendshapeKey::HEADDOWN, "headDown"},
//         {BlendshapeKey::HEADROLLLEFT, "headRollLeft"},
//         {BlendshapeKey::HEADROLLRIGHT, "headRollRight"},
//         {BlendshapeKey::EYEBLINKLEFT, "EyeBlinkLeft"},
//         {BlendshapeKey::EYEBLINKRIGHT, "EyeBlinkRight"},
//         // Add all other mappings here
//     };

//     for (const auto& testCase : testCases) {
//         QString result = blendshapeKeyToString(testCase.key);
//         EXPECT_EQ(result, testCase.expectedString)
//             << "Failed for BlendshapeKey: " << static_cast<int>(testCase.key);
//     }
// }

// TEST(BlendshapeKeyToStringTest, InvalidKey) {
//     // Assuming BlendshapeKey::UNKNOWN exists
//     BlendshapeKey invalidKey = BlendshapeKey::UNKNOWN;
//     QString result = blendshapeKeyToString(invalidKey);
//     EXPECT_TRUE(result.isEmpty()) << "Expected empty QString for UNKNOWN BlendshapeKey";
// }

// // Test Suite for blendshapeKeyFromString
// TEST(BlendshapeKeyFromStringTest, ValidStrings) {
//     struct TestCase {
//         QString str;
//         BlendshapeKey expectedKey;
//     };

//     // Define all valid string-key pairs
//     TestCase testCases[] = {
//         {"browOuterUp_L", BlendshapeKey::BROWOUTERUP_L},
//         {"browInnerUp_L", BlendshapeKey::BROWINNERUP_L},
//         {"browDown_L", BlendshapeKey::BROWDOWN_L},
//         {"eyeBlink_L", BlendshapeKey::EYEBLINK_L},
//         {"eyeSquint_L", BlendshapeKey::EYESQUINT_L},
//         {"eyeWide_L", BlendshapeKey::EYEWIDE_L},
//         {"eyeLookUp_L", BlendshapeKey::EYELOOKUP_L},
//         {"eyeLookOut_L", BlendshapeKey::EYELOOKOUT_L},
//         {"eyeLookIn_L", BlendshapeKey::EYELOOKIN_L},
//         {"eyeLookDown_L", BlendshapeKey::EYELOOKDOWN_L},
//         {"noseSneer_L", BlendshapeKey::NOSESNEER_L},
//         {"mouthUpperUp_L", BlendshapeKey::MOUTHUPPERUP_L},
//         {"mouthSmile_L", BlendshapeKey::MOUTHSMILE_L},
//         {"mouthLeft", BlendshapeKey::MOUTHLEFT},
//         {"mouthFrown_L", BlendshapeKey::MOUTHFROWN_L},
//         {"mouthLowerDown_L", BlendshapeKey::MOUTHLOWERDOWN_L},
//         {"jawLeft", BlendshapeKey::JAWLEFT},
//         {"cheekPuff", BlendshapeKey::CHEEKPUFF},
//         {"mouthShrugUpper", BlendshapeKey::MOUTHSHRUGUPPER},
//         {"mouthFunnel", BlendshapeKey::MOUTHFUNNEL},
//         {"mouthRollLower", BlendshapeKey::MOUTHROLLLOWER},
//         {"jawOpen", BlendshapeKey::JAWOPEN},
//         {"tongueOut", BlendshapeKey::TONGUEOUT},
//         {"mouthPucker", BlendshapeKey::MOUTHPUCKER},
//         {"mouthRollUpper", BlendshapeKey::MOUTHROLLUPPER},
//         {"jawRight", BlendshapeKey::JAWRIGHT},
//         {"mouthLowerDown_R", BlendshapeKey::MOUTHLOWERDOWN_R},
//         {"mouthFrown_R", BlendshapeKey::MOUTHFROWN_R},
//         {"mouthRight", BlendshapeKey::MOUTHRIGHT},
//         {"mouthSmile_R", BlendshapeKey::MOUTHSMILE_R},
//         {"mouthUpperUp_R", BlendshapeKey::MOUTHUPPERUP_R},
//         {"noseSneer_R", BlendshapeKey::NOSESNEER_R},
//         {"eyeLookDown_R", BlendshapeKey::EYELOOKDOWN_R},
//         {"eyeLookIn_R", BlendshapeKey::EYELOOKIN_R},
//         {"eyeLookOut_R", BlendshapeKey::EYELOOKOUT_R},
//         {"eyeLookUp_R", BlendshapeKey::EYELOOKUP_R},
//         {"eyeWide_R", BlendshapeKey::EYEWIDE_R},
//         {"eyeSquint_R", BlendshapeKey::EYESQUINT_R},
//         {"eyeBlink_R", BlendshapeKey::EYEBLINK_R},
//         {"browDown_R", BlendshapeKey::BROWDOWN_R},
//         {"browInnerUp_R", BlendshapeKey::BROWINNERUP_R},
//         {"browOuterUp_R", BlendshapeKey::BROWOUTERUP_R},
//         {"headLeft", BlendshapeKey::HEADLEFT},
//         {"headRight", BlendshapeKey::HEADRIGHT},
//         {"headUp", BlendshapeKey::HEADUP},
//         {"headDown", BlendshapeKey::HEADDOWN},
//         {"headRollLeft", BlendshapeKey::HEADROLLLEFT},
//         {"headRollRight", BlendshapeKey::HEADROLLRIGHT},
//         {"EyeBlinkLeft", BlendshapeKey::EYEBLINKLEFT},
//         {"EyeBlinkRight", BlendshapeKey::EYEBLINKRIGHT},
//         // Add all other mappings here
//     };

//     for (const auto& testCase : testCases) {
//         BlendshapeKey result = blendshapeKeyFromString(testCase.str);
//         EXPECT_EQ(result, testCase.expectedKey)
//             << "Failed for QString: " << testCase.str.toStdString();
//     }
// }

// TEST(BlendshapeKeyFromStringTest, InvalidString) {
//     QString invalidStr1 = "invalid_key";
//     QString invalidStr2 = "EyeBlink_L"; // Case-sensitive check
//     QString invalidStr3 = "";

//     EXPECT_EQ(blendshapeKeyFromString(invalidStr1), BlendshapeKey::UNKNOWN)
//         << "Expected UNKNOWN for invalid string: " << invalidStr1.toStdString();
//     EXPECT_EQ(blendshapeKeyFromString(invalidStr2), BlendshapeKey::UNKNOWN)
//         << "Expected UNKNOWN for case-sensitive invalid string: " << invalidStr2.toStdString();
//     EXPECT_EQ(blendshapeKeyFromString(invalidStr3), BlendshapeKey::UNKNOWN)
//         << "Expected UNKNOWN for empty string";
// }

// // Optional: Test Bidirectional Consistency
// TEST(BlendshapeKeyConsistencyTest, ToStringAndBack) {
//     struct TestCase {
//         BlendshapeKey key;
//     };

//     TestCase testCases[] = {
//         {BlendshapeKey::BROWOUTERUP_L},
//         {BlendshapeKey::BROWINNERUP_L},
//         {BlendshapeKey::BROWDOWN_L},
//         {BlendshapeKey::EYEBLINK_L},
//         {BlendshapeKey::EYESQUINT_L},
//         {BlendshapeKey::EYEWIDE_L},
//         {BlendshapeKey::EYELOOKUP_L},
//         {BlendshapeKey::EYELOOKOUT_L},
//         {BlendshapeKey::EYELOOKIN_L},
//         {BlendshapeKey::EYELOOKDOWN_L},
//         {BlendshapeKey::NOSESNEER_L},
//         {BlendshapeKey::MOUTHUPPERUP_L},
//         {BlendshapeKey::MOUTHSMILE_L},
//         {BlendshapeKey::MOUTHLEFT},
//         {BlendshapeKey::MOUTHFROWN_L},
//         {BlendshapeKey::MOUTHLOWERDOWN_L},
//         {BlendshapeKey::JAWLEFT},
//         {BlendshapeKey::CHEEKPUFF},
//         {BlendshapeKey::MOUTHSHRUGUPPER},
//         {BlendshapeKey::MOUTHFUNNEL},
//         {BlendshapeKey::MOUTHROLLLOWER},
//         {BlendshapeKey::JAWOPEN},
//         {BlendshapeKey::TONGUEOUT},
//         {BlendshapeKey::MOUTHPUCKER},
//         {BlendshapeKey::MOUTHROLLUPPER},
//         {BlendshapeKey::JAWRIGHT},
//         {BlendshapeKey::MOUTHLOWERDOWN_R},
//         {BlendshapeKey::MOUTHFROWN_R},
//         {BlendshapeKey::MOUTHRIGHT},
//         {BlendshapeKey::MOUTHSMILE_R},
//         {BlendshapeKey::MOUTHUPPERUP_R},
//         {BlendshapeKey::NOSESNEER_R},
//         {BlendshapeKey::EYELOOKDOWN_R},
//         {BlendshapeKey::EYELOOKIN_R},
//         {BlendshapeKey::EYELOOKOUT_R},
//         {BlendshapeKey::EYELOOKUP_R},
//         {BlendshapeKey::EYEWIDE_R},
//         {BlendshapeKey::EYESQUINT_R},
//         {BlendshapeKey::EYEBLINK_R},
//         {BlendshapeKey::BROWDOWN_R},
//         {BlendshapeKey::BROWINNERUP_R},
//         {BlendshapeKey::BROWOUTERUP_R},
//         {BlendshapeKey::HEADLEFT},
//         {BlendshapeKey::HEADRIGHT},
//         {BlendshapeKey::HEADUP},
//         {BlendshapeKey::HEADDOWN},
//         {BlendshapeKey::HEADROLLLEFT},
//         {BlendshapeKey::HEADROLLRIGHT},
//         {BlendshapeKey::EYEBLINKLEFT},
//         {BlendshapeKey::EYEBLINKRIGHT},
//         // Add all other keys here
//     };

//     for (const auto& testCase : testCases) {
//         QString str = blendshapeKeyToString(testCase.key);
//         BlendshapeKey key = blendshapeKeyFromString(str);
//         EXPECT_EQ(key, testCase.key)
//             << "Mismatch in bidirectional mapping for key: " << static_cast<int>(testCase.key);
//     }
// }

// TEST(BlendshapeKeyConsistencyTest, FromStringAndBack) {
//     struct TestCase {
//         QString str;
//     };

//     TestCase testCases[] = {
//         {"browOuterUp_L"},
//         {"browInnerUp_L"},
//         {"browDown_L"},
//         {"eyeBlink_L"},
//         {"eyeSquint_L"},
//         {"eyeWide_L"},
//         {"eyeLookUp_L"},
//         {"eyeLookOut_L"},
//         {"eyeLookIn_L"},
//         {"eyeLookDown_L"},
//         {"noseSneer_L"},
//         {"mouthUpperUp_L"},
//         {"mouthSmile_L"},
//         {"mouthLeft"},
//         {"mouthFrown_L"},
//         {"mouthLowerDown_L"},
//         {"jawLeft"},
//         {"cheekPuff"},
//         {"mouthShrugUpper"},
//         {"mouthFunnel"},
//         {"mouthRollLower"},
//         {"jawOpen"},
//         {"tongueOut"},
//         {"mouthPucker"},
//         {"mouthRollUpper"},
//         {"jawRight"},
//         {"mouthLowerDown_R"},
//         {"mouthFrown_R"},
//         {"mouthRight"},
//         {"mouthSmile_R"},
//         {"mouthUpperUp_R"},
//         {"noseSneer_R"},
//         {"eyeLookDown_R"},
//         {"eyeLookIn_R"},
//         {"eyeLookOut_R"},
//         {"eyeLookUp_R"},
//         {"eyeWide_R"},
//         {"eyeSquint_R"},
//         {"eyeBlink_R"},
//         {"browDown_R"},
//         {"browInnerUp_R"},
//         {"browOuterUp_R"},
//         {"headLeft"},
//         {"headRight"},
//         {"headUp"},
//         {"headDown"},
//         {"headRollLeft"},
//         {"headRollRight"},
//         {"EyeBlinkLeft"},
//         {"EyeBlinkRight"},
//         // Add all other strings here
//     };

//     for (const auto& testCase : testCases) {
//         BlendshapeKey key = blendshapeKeyFromString(testCase.str);
//         QString str = blendshapeKeyToString(key);
//         EXPECT_EQ(str, testCase.str)
//             << "Mismatch in bidirectional mapping for string: " << testCase.str.toStdString();
//     }
// }
