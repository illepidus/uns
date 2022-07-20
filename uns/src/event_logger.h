#pragma once
#include "global.h"
#include <QFile>

#define XML_HEADER               QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<?xml-stylesheet type=\"text/xsl\" href=\"uns_log.xsl\" ?>\n<EVENT_LOG gas=\"CO2\" date=\"%1\">\n").arg(DATE)
#define XML_FOOTER               QString("</EVENT_LOG>\n")
#define XML_EVENT_HEADER_PATTERN QString("\t<%1 time=\"%2\">\n")
#define XML_EVENT_FOOTER         QString("\t</%1>\n")
#define XML_DATA_PATTERN         QString("\t\t<%1>%2</%1>\n")
#define TMP_EVENT_LOG_PATH       QString("/tmp/uns_tmp.xml")

class EventLogger : public QObject {
	Q_OBJECT
	public slots:
		static void detonate();
		static int log(QString, QVariantMap);
};