#ifndef Song_h_
#define Song_h_

#include <QTextStream>
#include <QXmlStreamReader>
#include <QVector>

#include "enums.h"

struct BuffGroup;

struct Song {
	struct BuffRef {
		BuffRef() : buffGroup(NULL), level(1) {};
		QString id;
		BuffGroup *buffGroup;
		int level;
	};

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
