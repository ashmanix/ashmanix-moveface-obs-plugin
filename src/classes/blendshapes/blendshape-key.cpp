#include "blendshape-key.h"

BlendshapeKey blendshapeKeyFromString(const QString &str)
{
	if (auto it = blendshapeMapping.find(str); it != blendshapeMapping.end()) {
		return it->second;
	}
	return BlendshapeKey::UNKNOWN;
}

QString blendshapeKeyToString(BlendshapeKey key)
{
	// Create a reverse mapping (key-to-string) on first use.
	static QHash<BlendshapeKey, QString> reverseMapping = []() {
		QHash<BlendshapeKey, QString> map;
		// Iterate over the central mapping. For each unique key, pick the first string found.
		for (const auto &entry : blendshapeMapping) {
			if (!map.contains(entry.second)) {
				map.insert(entry.second, entry.first);
			}
		}
		return map;
	}();

	return reverseMapping.value(key, QString());
}
