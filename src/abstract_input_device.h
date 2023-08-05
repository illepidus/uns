#pragma once
#include "global.h"
#include "abstract_device.h"

class AbstractInputDevice : public AbstractDevice
{
	Q_OBJECT

	public:
		AbstractInputDevice(QString);
	signals:
		void newData(QString);
		void newData(QChar);
		void newData(QChar, QString);
		void newData(int);
	public slots:
		int start();
		int stop();
	protected slots:
		virtual int hardwareInit();
};