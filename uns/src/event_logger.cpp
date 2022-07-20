#include "event_logger.h"

void EventLogger::detonate()
{
	qDebug() << "EventLogger::detonate()";
}

int EventLogger::log(QString n, QVariantMap d)
{
	QFile tmp_file(TMP_EVENT_LOG_PATH);
	QString event_log_path = EVENT_LOG_PATH;
	if (!tmp_file.open(QIODevice::WriteOnly | QIODevice::Text))
		return ERROR_CODE::ACTION_PERMT;
	QTextStream out(&tmp_file);
	out << XML_HEADER;

	//++++++++++++
	QFile file(event_log_path);
	if (file.exists()) {
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return ERROR_CODE::ACTION_PERMT;
		QTextStream in(&file);
		in.seek(XML_HEADER.size());
		QString line = in.readLine();
		while (!line.isNull() && (line.trimmed() != XML_FOOTER.trimmed())) {
			out << line << "\n";
			line = in.readLine();
		}
	}
	//++++++++++++

	QMapIterator<QString, QVariant> i(d);
	out << XML_EVENT_HEADER_PATTERN.arg(n).arg(TIME);
	while (i.hasNext()) {
		i.next();
		out << XML_DATA_PATTERN.arg(i.key()).arg(i.value().toString());
	}
	out << XML_EVENT_FOOTER.arg(n);
	out << XML_FOOTER;

	if (file.exists()) 
		file.remove();

	if (!tmp_file.rename(event_log_path))
		return ERROR_CODE::ACTION_PERMT;

	return ERROR_CODE::NO_ERROR;
}