#include "Song.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include "QtCompatibility.h"

Song::Song() :
	duration(0), durationMaestro(0),
	extension(0), extensionMaestro(0)
{
}

void Song::print(QTextStream &stream, QString indent) const {
	stream << indent << "- notes: [";
	for (int i = 0; i < notes.count(); ++i) {
		if (i > 0) stream << ", ";
		stream << toString(notes[i]);
	}
	stream << "]" << endl;
	stream << indent << "- duration: " << duration << endl;
	stream << indent << "- duration_maestro: " << durationMaestro << endl;
	stream << indent << "- extension: " << extension << endl;
	stream << indent << "- extension_maestro: " << extensionMaestro << endl;
	foreach(const BuffRef &buff_ref, buffRefs) {
		stream << indent << "- buff ref: " << buff_ref.id <<
			"[" << buff_ref.level << "]";
		if (buff_ref.buffGroup) stream << " *";
		stream << endl;
	}
}

static void parse_buff_refs(QXmlStreamReader *xml, QVector<Song::BuffRef> *pout)
{
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			if (xml->name() == "buff_ref") {
				Song::BuffRef br;
				br.id = xml->attributes().value("id").toString();
				if (xml->attributes().hasAttribute("level")) {
					br.level = xml->attributes().value("level").toString().toInt();
				}
				pout->append(br);
			}
			XML_SKIP_CURRENT_ELEMENT(*xml);
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

static void parse_notes(QXmlStreamReader *xml, QVector<Note> *dnotes) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			for (int i = 0; i < NOTE_COUNT; ++i) {
				if (tag_name == toString((Note)i)) dnotes->append((Note)i);
			}
			XML_SKIP_CURRENT_ELEMENT(*xml);
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}

void Song::readXml(QXmlStreamReader *xml) {
	while (!xml->atEnd()) {
		QXmlStreamReader::TokenType token_type = xml->readNext();
		if (token_type == QXmlStreamReader::StartElement) {
			QStringRef tag_name = xml->name();
			if (tag_name == "duration") {
				duration = xml->readElementText().toDouble();
			} else if (tag_name == "duration_maestro") {
				durationMaestro = xml->readElementText().toDouble();
			} else if (tag_name == "extension") {
				extension = xml->readElementText().toDouble();
			} else if (tag_name == "extension_maestro") {
				extensionMaestro = xml->readElementText().toDouble();
			} else if (tag_name == "notes") {
				parse_notes(xml, &notes);
			} else if (tag_name == "buff_refs") {
				parse_buff_refs(xml, &buffRefs);
			} else {
				XML_SKIP_CURRENT_ELEMENT(*xml);
			}
		} else if (token_type == QXmlStreamReader::EndElement) {
			break;
		}
	}
}
