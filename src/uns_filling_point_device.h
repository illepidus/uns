#pragma once
#include "global.h"
#include "abstract_device.h"
#include "pi_button_device.h"
#include "pi_power_device.h"
#include "pi_matrix_keyboard_device.h"
#include "uns_unit_device.h"
#include "modbus_device.h"
#include <signal.h>

class UNSFillingPointDevice : public AbstractDevice
{
	Q_OBJECT
	public:
		UNSFillingPointDevice(QString, UNSUnitDevice*, PiMatrixKeyboardDevice*, PiButtonDevice*, PiButtonDevice*, PiButtonDevice*, PiPowerDevice*);
	signals:
		void newActivePump(int);
		void newReservoirTare(int, int);
		void fillingComplete(UNSFillingPointDevice *);
	public slots:
		int start();
		int stop();
		int processKeyboardCancel();
		int processKeyboardCommand(QString);
		int processKeyboardPress(QChar, QString);
		int processStopButtonClick();
		int processStartButtonClick();
		int processTareButtonClick();
		int processUnitData();
		int processUnitStateChange();
		int menuError(int);
		int menuDone();
		int buttonError(int);
		int buttonMessage(QString msg, int time = 0);
		int processEquipTimerTimeout();
	public:
		UNSUnitDevice *uns_unit;
		PiMatrixKeyboardDevice *keyboard;
		PiButtonDevice *stop_button;
		PiButtonDevice *start_button;
		PiButtonDevice *tare_button;
		PiPowerDevice *led;
		QTimer *equip_timer;
		QString menu_position;
		quint32 calibration_zero_value;
		quint32 calibration_span_value;
		int button_message_display_time;
		int button_error_display_time;
		int equip_timer_interval;
		QString tank_type;
		QTime fill_start;
		QTime fill_end;
		float fill_mass;
		float fill_tare;
		QString fill_mode;
		float calibration_span_weight;
		float zero_weight_delta;
		float empty_tank_weight;
		float empty_tank_weight_temp;
		float empty_tank_weight_delta;
		float gas_weight;
		float gas_weight_temp;
		float min_empty_tank_weight;
		float max_empty_tank_weight;
		float min_gas_weight;
		float max_gas_weight;
		float default_empty_tank_weight_delta;
		float rack_weight;

		bool overweight_suss; //Подозрение на переполнение
};