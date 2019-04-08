#ifndef QTCOMPATIBILITY_H_
#define QTCOMPATIBILITY_H_

#if (QT_VERSION >= 0x040600)
#define XML_SKIP_CURRENT_ELEMENT(x) ((x).skipCurrentElement())
#else
#define XML_SKIP_CURRENT_ELEMENT(x) do { \
	QXmlStreamReader &macro_var_xml = (x); \
	int macro_var_depth = 1; \
	while (macro_var_depth && macro_var_xml.readNext() != QXmlStreamReader::Invalid) { \
		if (macro_var_xml.isEndElement()) { \
			--macro_var_depth; \
		} else if (macro_var_xml.isStartElement()) { \
			++macro_var_depth; \
		} \
	} \
} while (0)
#endif

#endif
