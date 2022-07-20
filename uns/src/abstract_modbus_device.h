#pragma once
#include "global.h"
#include "abstract_device.h"
#include "modbus_device.h"

class AbstractModbusDevice : public AbstractDevice
{
	Q_OBJECT
	public:
		AbstractModbusDevice(QString, ModbusDevice *);
	signals:
		void newData();
	public slots:
		int start();
		int stop();
		int request(QByteArray); //raw data unit with fn code and CRC
		int request(int, QByteArray);
		int addRoutine(QByteArray); //raw data unit with fn code and CRC
		int addRoutine(int, QByteArray);
	protected slots:
		virtual int hardwareInit();
		virtual int hardwareProcessDataUnit(QByteArray, QByteArray);
		int processDataUnit(QByteArray, QByteArray);
		int hangUp();
	protected:
		ModbusDevice *modbus;
		QTimer *request_timer;
		QTimer *hang_timer;
		int modbus_id;
		int request_interval;
		int hang_interval;
		bool disabled;
};