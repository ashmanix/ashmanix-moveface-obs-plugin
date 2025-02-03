#ifndef MOVEFACESOURCEPRIVATE_H
#define MOVEFACESOURCEPRIVATE_H

#include <QMap>
#include <QMutex>
#include <QMutexLocker>

// Forward-declare OBS source and your private type
struct obs_source;
struct moveface_image_source;

// Global map for storing our private data pointers keyed by obs_source pointer
inline QMap<obs_source *, moveface_image_source *> g_sourceDataMap;
inline QMutex g_sourceDataMutex;

// Helper function to retrieve our private data
inline moveface_image_source *getMyPrivateData(obs_source *source)
{
	QMutexLocker locker(&g_sourceDataMutex);
	return g_sourceDataMap.value(source, nullptr);
}

#endif // MOVEFACESOURCEPRIVATE_H
