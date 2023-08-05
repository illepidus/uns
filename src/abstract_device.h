#pragma once
#include "global.h"
#include <QSettings>

class AbstractDevice : public QObject
{
	Q_OBJECT

	public:
		AbstractDevice(QString);
	signals:
		void stateChanged();
		void stateChanged(int);
		void settingSaved();
		void dataSaved();
	public slots:
		virtual int start();
		virtual int stop();

		int getState();
		int getPreviousState();
		int setState(int);
		virtual int detonate(); //for testing purposes

		QString getName();
		QString getClass();
		QVariantList getSlots();
		static QString getStateName(int);

		void dbg(QString);

	protected slots:
		int setSetting(QString, QVariant);
		int setData(QString, QVariant);
		int loadSettings();
		QVariant getSetting(QString);
		QVariant getData(QString);
	protected:
		QSettings *device_settings;
		QSettings *device_data;
		QString device_class;
		QString device_name;
		QString device_description;
		int state;
		int previous_state;
};