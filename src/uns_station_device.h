#pragma once

#include "global.h"
#include "abstract_device.h"
#include "uns_filling_point_device.h"
#include "sb_unit_device.h"
#include "pi_power_device.h"
#include "event_logger.h"

class UNSStationDevice : public AbstractDevice
{
	Q_OBJECT
	public:
		UNSStationDevice(QString);
	public slots:
		int start();
		int stop();
		int processStateChanges();
		int setActivePump(int);
		int setReservoirTare(int, int);
		int logFill(UNSFillingPointDevice *);
		int setHourTimer();
		int logHour();
	private:
		ModbusDevice  *modbus_1;
		ModbusDevice  *modbus_2;
		PiPowerDevice *pump_1;
		PiPowerDevice *pump_2;
		SBUnitDevice  *sb_unit_1;
		SBUnitDevice  *sb_unit_2;
		UNSUnitDevice *uns_unit_1;
		PiMatrixKeyboardDevice *keyboard_1;
		PiButtonDevice *stop_button_1;
		PiButtonDevice *start_button_1;
		PiButtonDevice *tare_button_1;
		PiPowerDevice  *led_1;
		UNSFillingPointDevice *point_1;
		UNSUnitDevice *uns_unit_2;
		PiMatrixKeyboardDevice *keyboard_2;
		PiButtonDevice *stop_button_2;
		PiButtonDevice *start_button_2;
		PiButtonDevice *tare_button_2;
		PiPowerDevice  *led_2;
		UNSFillingPointDevice *point_2;

		int active_pump;
		QTimer *hour_timer;
};