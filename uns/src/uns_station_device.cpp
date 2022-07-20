#include "uns_station_device.h"

UNSStationDevice::UNSStationDevice(QString n) : AbstractDevice(n)
{
	pump_1 = new PiPowerDevice("PUMP_1"); pump_1->start();
	pump_2 = new PiPowerDevice("PUMP_2"); pump_2->start();

	modbus_1 = new ModbusDevice("MODBUS_1"); modbus_1->start();

	uns_unit_1 = new UNSUnitDevice("UNS_UNIT_1", modbus_1); uns_unit_1->start();
	keyboard_1 = new PiMatrixKeyboardDevice("KEYBOARD_1"); keyboard_1->start();
	stop_button_1  = new PiButtonDevice("STOP_BUTTON_1"); stop_button_1->start();
	start_button_1 = new PiButtonDevice("START_BUTTON_1"); start_button_1->start();
	tare_button_1  = new PiButtonDevice("TARE_BUTTON_1"); tare_button_1->start();
	led_1  = new PiPowerDevice("LED_1"); led_1->start();
	point_1 = new UNSFillingPointDevice("POINT_1", uns_unit_1, keyboard_1, stop_button_1, start_button_1, tare_button_1, led_1); point_1->start();

	uns_unit_2 = new UNSUnitDevice("UNS_UNIT_2", modbus_1); uns_unit_2->start();
	keyboard_2 = new PiMatrixKeyboardDevice("KEYBOARD_2"); keyboard_2->start();
	stop_button_2  = new PiButtonDevice("STOP_BUTTON_2"); stop_button_2->start();
	start_button_2 = new PiButtonDevice("START_BUTTON_2"); start_button_2->start();
	tare_button_2  = new PiButtonDevice("TARE_BUTTON_2"); tare_button_2->start();
	led_2  = new PiPowerDevice("LED_2"); led_2->start();
	point_2 = new UNSFillingPointDevice("POINT_2", uns_unit_2, keyboard_2, stop_button_2, start_button_2, tare_button_2, led_2); point_2->start();

	modbus_2 = new ModbusDevice("MODBUS_2"); modbus_2->start();
	sb_unit_1 = new SBUnitDevice("SB_UNIT_1", modbus_2); sb_unit_1->start();
	sb_unit_2 = new SBUnitDevice("SB_UNIT_2", modbus_2); sb_unit_2->start();

	hour_timer = new QTimer();
}

int UNSStationDevice::start()
{
	active_pump = 1;
	if (getData("active_pump").isValid())
		active_pump = getData("active_pump").toInt();
	else 
		setActivePump(active_pump);

	QObject::connect(point_1, SIGNAL(stateChanged()), this, SLOT(processStateChanges()));
	QObject::connect(point_2, SIGNAL(stateChanged()), this, SLOT(processStateChanges()));
	QObject::connect(point_1, SIGNAL(newActivePump(int)), this, SLOT(setActivePump(int)));
	QObject::connect(point_2, SIGNAL(newActivePump(int)), this, SLOT(setActivePump(int)));
	QObject::connect(point_1, SIGNAL(fillingComplete(UNSFillingPointDevice *)), this, SLOT(logFill(UNSFillingPointDevice *)));
	QObject::connect(point_2, SIGNAL(fillingComplete(UNSFillingPointDevice *)), this, SLOT(logFill(UNSFillingPointDevice *)));
	QObject::connect(hour_timer, SIGNAL(timeout()), this, SLOT(logHour()));
	QObject::connect(point_1, SIGNAL(newReservoirTare(int, int)), this, SLOT(setReservoirTare(int, int)));
	QObject::connect(point_2, SIGNAL(newReservoirTare(int, int)), this, SLOT(setReservoirTare(int, int)));

	setHourTimer();
	setState(DEVICE_STATE::ON);

	return ERROR_CODE::NO_ERROR;
}

int UNSStationDevice::stop()
{
	QObject::disconnect(point_1, SIGNAL(stateChanged()), this, SLOT(processStateChanges()));
	QObject::disconnect(point_2, SIGNAL(stateChanged()), this, SLOT(processStateChanges()));
	QObject::disconnect(point_1, SIGNAL(newActivePump(int)), this, SLOT(setActivePump(int)));
	QObject::disconnect(point_2, SIGNAL(newActivePump(int)), this, SLOT(setActivePump(int)));
	QObject::disconnect(point_1, SIGNAL(fillingComplete(UNSFillingPointDevice *)), this, SLOT(logFill(UNSFillingPointDevice *)));
	QObject::disconnect(point_2, SIGNAL(fillingComplete(UNSFillingPointDevice *)), this, SLOT(logFill(UNSFillingPointDevice *)));
	QObject::disconnect(hour_timer, SIGNAL(timeout()), this, SLOT(logHour()));
	QObject::disconnect(point_1, SIGNAL(newReservoirTare(int, int)), this, SLOT(setReservoirTare(int, int)));
	QObject::disconnect(point_2, SIGNAL(newReservoirTare(int, int)), this, SLOT(setReservoirTare(int, int)));

	setState(DEVICE_STATE::OFF);
	return ERROR_CODE::NO_ERROR;
}

int UNSStationDevice::processStateChanges()
{
	bool pump_on = false;

	if (point_1->getState() == DEVICE_STATE::READY ||
		point_2->getState() == DEVICE_STATE::READY)
	pump_on = true;

	if ((point_1->getState() == DEVICE_STATE::OVERWEIGHT) || 
		(point_2->getState() == DEVICE_STATE::OVERWEIGHT) || 
		(point_1->getState() == DEVICE_STATE::UNKNOWN) || 
		(point_2->getState() == DEVICE_STATE::UNKNOWN) || 
		(point_1->getState() == DEVICE_STATE::STOPPED) || 
		(point_2->getState() == DEVICE_STATE::STOPPED) || 
		(point_1->getState() == DEVICE_STATE::HANGED) || 
		(point_2->getState() == DEVICE_STATE::HANGED) || 
		(point_1->getState() == DEVICE_STATE::ERROR) || 
		(point_2->getState() == DEVICE_STATE::ERROR))
	pump_on = false;

	if (pump_on) {
		led_1->switchOn();
		led_2->switchOn();
		if (active_pump == 2) {
			pump_1->switchOff();
			pump_2->switchOn();
			dbg("Включаем насос 2");
			return ERROR_CODE::NO_ERROR;
		}
		pump_1->switchOn();
		pump_2->switchOff();
		dbg("Включаем насос 1");
		return ERROR_CODE::NO_ERROR;
	}

	led_1->switchOff();
	led_2->switchOff();

	if (active_pump == 2) {
		if (pump_2->getState() == DEVICE_STATE::ON)
			dbg("Выключаем насос 2");
		pump_1->switchOff();
		pump_2->switchOff();
		return ERROR_CODE::NO_ERROR;
	}
	if (pump_1->getState() == DEVICE_STATE::ON)
		dbg("Выключаем насос 1");
	pump_1->switchOff();
	pump_2->switchOff();
	return ERROR_CODE::NO_ERROR;
}

int UNSStationDevice::setActivePump(int p)
{
	if ((p < 1) || (p > 2)) {
		dbg(QString("Невозможно сменить активный насос на [%1]!").arg(p));
		return ERROR_CODE::UNEXPD_ENTRY;
	}

	active_pump = p;
	setData("active_pump", p);

	if (p != active_pump)
		dbg(QString("Смена активного насоса: [%1]->[%2]").arg(active_pump).arg(p));

	return ERROR_CODE::NO_ERROR;
}


int UNSStationDevice::setReservoirTare(int r, int t)
{
	if (r == 1) {
		sb_unit_1->setTare(t);
		sb_unit_1->saveEEPROM();
		return ERROR_CODE::NO_ERROR;
	}

	if (r == 2) {
		sb_unit_2->setTare(t);
		sb_unit_2->saveEEPROM();
		return ERROR_CODE::NO_ERROR;
	}

	dbg(QString("Невозможно установить тару для резервуара [%1]!").arg(r));
	return ERROR_CODE::UNEXPD_ENTRY;
}


int UNSStationDevice::logFill(UNSFillingPointDevice * p)
{
	//Формат предложен Антоном. Все вопросы к нему.
	QVariantMap data;
	data.insert("POST", p->getName()); //POINT_1 | POINT_2
	data.insert("MODE", p->fill_mode); //BXXX
	data.insert("TARA", p->fill_tare); //Масса пустого баллона с накрученным вентилем
	data.insert("MASS", p->fill_mass); //Масса закачанного газа
	data.insert("TANK", active_pump);  //Имя емкости-источника CO2
	data.insert("TIME", p->fill_start.secsTo(p->fill_end)); //Время наполнения в секундах
	if (sb_unit_1->getState() == DEVICE_STATE::ON) data.insert("GAS1", (sb_unit_1->getMass() - sb_unit_1->getTare())); //Остаток газа в ёмкости 1
	if (sb_unit_2->getState() == DEVICE_STATE::ON) data.insert("GAS2", (sb_unit_2->getMass() - sb_unit_2->getTare())); //Остаток газа в ёмкости 2

	EventLogger::log("FILL", data);
	return ERROR_CODE::NO_ERROR;
}

int UNSStationDevice::logHour()
{
	setHourTimer();

	QVariantMap data;
	if (sb_unit_1->getState() == DEVICE_STATE::ON) data.insert("GAS1", (sb_unit_1->getMass() - sb_unit_1->getTare()));
	if (sb_unit_2->getState() == DEVICE_STATE::ON) data.insert("GAS2", (sb_unit_2->getMass() - sb_unit_2->getTare()));

	EventLogger::log("HOUR", data);
	return ERROR_CODE::NO_ERROR;
}

int UNSStationDevice::setHourTimer()
{
	hour_timer->setSingleShot(true);
	QTime n; n = QTime::currentTime();
	int h = n.hour();
	if ((n.minute() == 59) && (n.second() == 59)) h += 1;
	h = (h < 23) ? (h + 1) : (0);
	QTime t; t.setHMS(h, 0, 1);
	int ms = n.msecsTo(t);
	if (ms < 0) ms += 86400000;
	hour_timer->start(ms);

	return ERROR_CODE::NO_ERROR;
}
