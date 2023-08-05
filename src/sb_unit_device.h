#pragma once
#include "global.h"
#include "abstract_modbus_device.h"

#define MINIMUM_SB_UNIT_UPDATE_INTERVAL 50
#define MAXIMUM_SB_UNIT_UPDATE_INTERVAL 2000
#define MINIMUM_SB_UNIT_BRIGHTNESS 0
#define MAXIMUM_SB_UNIT_BRIGHTNESS 15

class SBUnitDevice : public AbstractModbusDevice
{
	Q_OBJECT
	public:
		SBUnitDevice(QString, ModbusDevice *);
	signals:
		void newData(float);
	private slots:
		int hardwareInit();
		int hardwareProcessDataUnit(QByteArray, QByteArray);
		int setDeviceMode(quint16);
	public slots:
		int saveEEPROM();
		int setTare(float);
		int setTareCurrent();
		int setTareZero();
		int setCalibration(float, float, float, float, float, float, float, float); //(a, b): w = ax + b
		int setUpdateInterval(quint16);
		int setDisplayBrightness(quint16);
		int detonate();
		float getMass();
		float getTare();
	private:
		float mass;
		float tare;

		quint16 device_mode;
		quint16 update_interval;
		quint16 hits_per_weight;
		float scales1_intercept;
		float scales1_slope;
		float scales2_intercept;
		float scales2_slope;
		float scales3_intercept;
		float scales3_slope;
		float scales4_intercept;
		float scales4_slope;
		QString mass_unit;
		quint16 display_brightness;
		quint16 uart_baudrate;
		quint16 rs485_baudrate;
		quint16 rs485_timeout;

		QByteArray routine_request_du;
		QByteArray initial_request_du;
		QByteArray device_mode_request_du;
		QByteArray display_brightness_request_du;
		QByteArray update_interval_request_du;
		QByteArray calibration_request_du;
		QByteArray eeprom_request_du;
		QByteArray tare_request_du;
};