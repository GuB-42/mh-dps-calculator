#ifndef Song_h_
#define Song_h_

#include <QTextStream>
#include <QXmlStreamReader>
#include <QVector>

#include "enums.h"
#include "BuffRef.h"

struct BuffGroup;

struct Song {
	QVector<Note> notes;
	double duration;
	double durationMaestro;
	double extension;
	double extensionMaestro;
	QVector<BuffRef> buffRefs;

	Song();
	void print(QTextStream &stream, QString indent = QString()) const;
	void readXml(QXmlStreamReader *xml);
};

#endif
