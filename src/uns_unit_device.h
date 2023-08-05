#pragma once
#include "global.h"
#include "abstract_modbus_device.h"

#define MINIMUM_UNS_UNIT_UPDATE_INTERVAL 50
#define MAXIMUM_UNS_UNIT_UPDATE_INTERVAL 2000
#define MINIMUM_UNS_UNIT_STABLE_DELTA 0.1
#define MAXIMUM_UNS_UNIT_STABLE_DELTA 10
#define MINIMUM_UNS_UNIT_STABLE_COUNT 3
#define MAXIMUM_UNS_UNIT_STABLE_COUNT 32
#define MINIMUM_UNS_UNIT_BRIGHTNESS 0
#define MAXIMUM_UNS_UNIT_BRIGHTNESS 15

class UNSUnitDevice : public AbstractModbusDevice
{
	Q_OBJECT
	public:
		UNSUnitDevice(QString, ModbusDevice *);
	private slots:
		int hardwareInit();
		int hardwareProcessDataUnit(QByteArray, QByteArray);
	public slots:
		int detonate();
		int display(QString s, bool c=false);
		int displayWeight();
		int saveEEPROM();
		int setTare(float);
		int setTareCurrent();
		int setTareZero();
		int softwareReset();
		int setCalibration(float, float); //(a, b): w = ax + b
		int setMassStableDelta(float);
		int setMassStableCount(quint16);
		int setUpdateInterval(quint16);
		int setDisplayBrightness(quint16);
		int setActiveScales(int);
		int getActiveScales();
		int getDisplayMode();
	public:
		quint16 device_state;
		bool state_stable;
		bool state_rising;
		bool state_lowering;
		qint32 value;
		float mass;
		float tare;

		quint16 hardware_id;
		quint16 hardware_version;
		quint16 software_version;
		QString uns_device_name;
		QString uns_device_comment;
		quint16 device_mode;
		quint16 update_interval;
		quint16 hits_per_weight;
		float   scales1_intercept;
		float   scales1_slope;
		float   scales2_intercept;
		float   scales2_slope;
		QString mass_unit;
		quint16 display_brightness;
		quint16 uart_baudrate;
		quint16 rs485_baudrate;
		quint16 rs485_timeout;
		quint16 mass_stable_count;
		float   mass_stable_delta;
		QString display_text;

		QByteArray routine_request_du;
		QByteArray initial_request_du;
		QByteArray active_scales_du;
		QByteArray display_request_du;
		QByteArray calibration_request_du;
		QByteArray eeprom_request_du;
		QByteArray tare_request_du;
		QByteArray mass_stable_delta_du;
		QByteArray mass_stable_count_du;
		QByteArray update_interval_du;
		QByteArray display_brightness_du;
		QByteArray reset_request_du;
};