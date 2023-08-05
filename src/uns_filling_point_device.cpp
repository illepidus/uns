#include "uns_filling_point_device.h"

UNSFillingPointDevice::UNSFillingPointDevice(QString n, UNSUnitDevice *u, PiMatrixKeyboardDevice *k, PiButtonDevice *b1, PiButtonDevice *b2, PiButtonDevice *b3, PiPowerDevice *l) : AbstractDevice(n)
{
	device_settings->beginGroup("UNS");
		foreach (const QString &key, device_settings->allKeys()) {
			setProperty(key.toUtf8(), device_settings->value(key));
		}
	device_settings->endGroup();

	uns_unit = u;
	keyboard = k;
	stop_button  = b1;
	start_button = b2;
	tare_button  = b3;
	led = l;

	equip_timer = new QTimer();

	QObject::connect(keyboard, SIGNAL(cancel()), this, SLOT(processKeyboardCancel()));
	QObject::connect(keyboard, SIGNAL(command(QString)), this, SLOT(processKeyboardCommand(QString)));
	QObject::connect(keyboard, SIGNAL(keyPress(QChar, QString)), this, SLOT(processKeyboardPress(QChar, QString)));
	QObject::connect(stop_button,  SIGNAL(clicked()), this, SLOT(processStopButtonClick()));
	QObject::connect(start_button, SIGNAL(clicked()), this, SLOT(processStartButtonClick()));
	QObject::connect(tare_button,  SIGNAL(clicked()), this, SLOT(processTareButtonClick()));
	QObject::connect(uns_unit,  SIGNAL(newData()), this, SLOT(processUnitData()));
	QObject::connect(uns_unit,  SIGNAL(stateChanged()), this, SLOT(processUnitStateChange()));
	QObject::connect(equip_timer,  SIGNAL(timeout()), this, SLOT(processEquipTimerTimeout()));
}

int UNSFillingPointDevice::start() {
	setState(DEVICE_STATE::UNKNOWN);
	dbg("[UNKNOWN] Инициализация.");

	overweight_suss = false;

	zero_weight_delta = 0.3;
	if (getSetting("zero_weight_delta").isValid())
		zero_weight_delta = getSetting("zero_weight_delta").toFloat();

	empty_tank_weight = 10000.0;
	if (getSetting("empty_tank_weight").isValid())
		empty_tank_weight = getSetting("empty_tank_weight").toFloat();

	empty_tank_weight_delta = 0.0;
	if (getSetting("empty_tank_weight_delta").isValid())
		empty_tank_weight_delta = getSetting("empty_tank_weight_delta").toFloat();

	gas_weight = 0.0;
	if (getSetting("gas_weight").isValid())
		gas_weight = getSetting("gas_weight").toFloat();

	min_empty_tank_weight = 1.0;
	if (getSetting("min_empty_tank_weight").isValid())
		min_empty_tank_weight = getSetting("min_empty_tank_weight").toFloat();

	max_empty_tank_weight = 100.0;
	if (getSetting("max_empty_tank_weight").isValid())
		max_empty_tank_weight = getSetting("max_empty_tank_weight").toFloat();

	min_gas_weight = 0.1;
	if (getSetting("min_gas_weight").isValid())
		min_gas_weight = getSetting("min_gas_weight").toFloat();

	max_gas_weight = 100.0;
	if (getSetting("max_gas_weight").isValid())
		max_gas_weight = getSetting("max_gas_weight").toFloat();

	default_empty_tank_weight_delta = 5.0;
	if (getSetting("default_empty_tank_weight_delta").isValid())
		default_empty_tank_weight_delta = getSetting("default_empty_tank_weight_delta").toFloat();

	button_error_display_time = 300;
	if (getSetting("button_error_display_time").isValid())
		button_error_display_time = getSetting("button_error_display_time").toInt();

	button_message_display_time = 300;
	button_message_display_time = button_error_display_time;
	if (getSetting("button_message_display_time").isValid())
		button_message_display_time = getSetting("button_message_display_time").toInt();

	rack_weight = 0.0;
	if (getSetting("rack_weight").isValid())
		rack_weight = getSetting("rack_weight").toFloat();

	equip_timer_interval = 10000;
	if (getSetting("equip_timer_interval").isValid())
		equip_timer_interval = getSetting("equip_timer_interval").toInt();
	equip_timer->setInterval(equip_timer_interval);
	equip_timer->setSingleShot(true);

	tank_type = "NOT_SELECTED";
	setData("tank_type", tank_type);

	return ERROR_CODE::NO_ERROR;
}

int UNSFillingPointDevice::stop()
{
	setState(DEVICE_STATE::STOPPED);
	buttonMessage("stop");
	return ERROR_CODE::NO_ERROR;
}

int UNSFillingPointDevice::processKeyboardCancel()
{
	menu_position.clear();
	if (getState() == DEVICE_STATE::STOPPED)
		return stop();
	uns_unit->displayWeight();
	return ERROR_CODE::NO_ERROR;
}

int UNSFillingPointDevice::processKeyboardCommand(QString s) {
	menu_position += "/";
	menu_position += s;

	QRegExp rx("");

	/******************ПУСТАЯ КОМАНДА******************/
	rx.setPattern("^/$");
	if (rx.indexIn(menu_position) == 0) {
		return processKeyboardCancel();
	}

	/**********ЗАПОЛНЕНИЕ УНИКАЛЬНЫХ БАЛЛОНОВ**********/
	rx.setPattern("^/B$");
	if (rx.indexIn(menu_position) == 0) {
		if (getState() != DEVICE_STATE::EMPTY)
			return menuError(ERROR_CODE::BAD_DV_STATE);
		uns_unit->display("bal");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/B/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		if (getState() != DEVICE_STATE::EMPTY)
			return menuError(ERROR_CODE::BAD_DV_STATE);
		empty_tank_weight_temp = rx.cap(1).toFloat() / 100;
		if ((empty_tank_weight_temp < min_empty_tank_weight) || empty_tank_weight_temp > max_empty_tank_weight)
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->display("gas");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/B/\\d+/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		if (getState() != DEVICE_STATE::EMPTY)
			return menuError(ERROR_CODE::BAD_DV_STATE);
		gas_weight_temp = rx.cap(1).toFloat() / 100;
		if ((gas_weight_temp < min_gas_weight) || gas_weight_temp > max_gas_weight)
			return menuError(ERROR_CODE::UNEXPD_ENTRY);

		rack_weight = 0.0;
		empty_tank_weight = empty_tank_weight_temp;
		gas_weight = gas_weight_temp;
		empty_tank_weight_delta = default_empty_tank_weight_delta;
		tank_type = "UNIQUE " + menu_position;
		setData("tank_type", tank_type);
		uns_unit->setTareZero();
		return menuDone();
	}

	/*******ЗАПОЛНЕНИЕ ПРЕДОПРЕДЕЛЕННЫХ БАЛЛОНОВ*******/
	rx.setPattern("^/(B\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		if (getState() != DEVICE_STATE::EMPTY)
			return menuError(ERROR_CODE::BAD_DV_STATE);
		if ((!getSetting(rx.cap(1) + ("/empty_tank_weight")).isValid()) || (!getSetting(rx.cap(1) + ("/empty_tank_weight_delta")).isValid()) || (!getSetting(rx.cap(1) + ("/gas_weight")).isValid()))
			return menuError(ERROR_CODE::BROKEN_ENTRY);
		empty_tank_weight       = getSetting(rx.cap(1) + "/empty_tank_weight").toFloat();
		empty_tank_weight_delta = getSetting(rx.cap(1) + "/empty_tank_weight_delta").toFloat();
		gas_weight              = getSetting(rx.cap(1) + "/gas_weight").toFloat();
		rack_weight             = getSetting(rx.cap(1) + "/rack_weight").isValid() ? getSetting(rx.cap(1) + "/rack_weight").toFloat() : 0.0;
		tank_type = rx.cap(1);
		setData("tank_type", tank_type);
		uns_unit->setTareZero();
		return menuDone();
	}

	/********Перезагрузка модуля без сохранения*********/
	rx.setPattern("^/C0$");
	if (rx.indexIn(menu_position) == 0) {
		menu_position = "";
		uns_unit->softwareReset();
		return ERROR_CODE::NO_ERROR;
	}

	/********Перезагрузка  модуля с сохранением*********/
	rx.setPattern("^/C1$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->displayWeight();
		uns_unit->saveEEPROM();
		menu_position = "";
		uns_unit->softwareReset();
		return ERROR_CODE::NO_ERROR;
	}

	/********************Калибровка********************/
	rx.setPattern("^/C2$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("null");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/C2/$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("span");
		calibration_zero_value = uns_unit->value;
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/C2//(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		calibration_span_weight = rx.cap(1).toFloat() / 100;
		calibration_span_value = uns_unit->value;
		if (calibration_zero_value >= calibration_span_value)
			return menuError(ERROR_CODE::BROKEN_ENTRY);
		float a = calibration_span_weight / (calibration_span_value - calibration_zero_value);
		float b = - calibration_span_weight * calibration_zero_value / (calibration_span_value - calibration_zero_value);
		uns_unit->setCalibration(a, b);
		uns_unit->setTareZero();
		return menuDone();
	}

	/**************Выбор активного насоса***************/
	rx.setPattern("^/C3$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("HCOC");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/C3/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < 1) || (cap > 2))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		emit newActivePump(cap);
		return menuDone();
	}

	/*****************Перезапуск демона*****************/
	rx.setPattern("^/D0$");
	if (rx.indexIn(menu_position) == 0) {
		menu_position = "";
		QCoreApplication::exit(SIGABRT);
		return ERROR_CODE::NO_ERROR;
	}
	
	/****************Перезагрузка сервера***************/
	rx.setPattern("^/D1$");
	if (rx.indexIn(menu_position) == 0) {
		menu_position = "";
		system("shutdown -r now");
		return ERROR_CODE::NO_ERROR;
	}


	/*********Калибровка без проверки на ошибки*********/
	rx.setPattern("^/D2$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("null");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D2/$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("span");
		calibration_zero_value = uns_unit->value;
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D2//(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		calibration_span_weight = rx.cap(1).toFloat() / 100;
		calibration_span_value = uns_unit->value;
		if (calibration_zero_value >= calibration_span_value)
			return menuError(ERROR_CODE::BROKEN_ENTRY);
		float a = calibration_span_weight / (calibration_span_value - calibration_zero_value);
		float b = - calibration_span_weight * calibration_zero_value / (calibration_span_value - calibration_zero_value);
		uns_unit->setCalibration(a, b);
		uns_unit->setTareZero();
		return menuDone();
	}

	/***************Выбор активных весов****************/
	rx.setPattern("^/D3$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("BEC");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D3/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < 1) || (cap > 2))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->setActiveScales(cap);

		return menuDone();
	}

	/*******Установка интервала обновления модуля*******/
	rx.setPattern("^/D4$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("int");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D4/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < MINIMUM_UNS_UNIT_UPDATE_INTERVAL) || (cap > MAXIMUM_UNS_UNIT_UPDATE_INTERVAL))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->setUpdateInterval(cap);

		return menuDone();
	}

	/**********Установка разбега стабильности***********/
	rx.setPattern("^/D5$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("delt");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D5/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		float cap = rx.cap(1).toFloat() / 100;
		if ((cap < MINIMUM_UNS_UNIT_STABLE_DELTA) || (cap > MAXIMUM_UNS_UNIT_STABLE_DELTA))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->setMassStableDelta(cap);

		return menuDone();
	}

	/******Установка размера истории стабильности********/
	rx.setPattern("^/D6$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("coun");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D6/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < MINIMUM_UNS_UNIT_STABLE_COUNT) || (cap > MAXIMUM_UNS_UNIT_STABLE_COUNT))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->setMassStableCount(cap);

		return menuDone();
	}

	/**********Установка яркости дисплея***************/
	rx.setPattern("^/D7$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("brig");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D7/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < MINIMUM_UNS_UNIT_BRIGHTNESS) || (cap > MAXIMUM_UNS_UNIT_BRIGHTNESS))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->setDisplayBrightness(cap);

		return menuDone();
	}

	/*************Установка тары ёмкости**************/
	rx.setPattern("^/D8$");
	if (rx.indexIn(menu_position) == 0) {
		uns_unit->display("tank");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D8/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap = rx.cap(1).toInt();
		if ((cap < 1) || (cap > 2))
			return menuError(ERROR_CODE::UNEXPD_ENTRY);
		uns_unit->display("tare");
		return ERROR_CODE::NO_ERROR;
	}

	rx.setPattern("^/D8/(\\d+)/(\\d+)$");
	if (rx.indexIn(menu_position) == 0) {
		int cap1 = rx.cap(1).toInt();
		int cap2 = rx.cap(2).toInt();

		emit newReservoirTare(cap1, cap2);
		return menuDone();
	}


	return menuError(ERROR_CODE::UNKNWN_ENTRY);
}

int UNSFillingPointDevice::buttonError(int e) {
	uns_unit->display(QString("E %1").arg(e, 2, 10, QChar('0')));
	QTimer::singleShot(button_error_display_time, uns_unit, SLOT(displayWeight()));
	return e;
}

int UNSFillingPointDevice::buttonMessage(QString m, int t) {
	int e = uns_unit->display(m);
	if (t > 0)
		QTimer::singleShot(t, uns_unit, SLOT(displayWeight()));
	return e;
}

int UNSFillingPointDevice::menuError(int e) {
	menu_position = "";
	uns_unit->display(QString("E %1").arg(e, 2, 10, QChar('0')));
	return e;
}

int UNSFillingPointDevice::menuDone() {
	menu_position = "";
	uns_unit->display("done");
	return ERROR_CODE::NO_ERROR;
}

int UNSFillingPointDevice::processKeyboardPress(QChar, QString s) {
	QRegExp rx;
	rx.setPattern("^(\\d+)$");
	if (rx.indexIn(s) < 0) {
		uns_unit->display(s);
		return ERROR_CODE::NO_ERROR;
	}
	uns_unit->display(QString("%1").arg(s.toInt(), 3, 10, QChar('0')), true);
	return ERROR_CODE::NO_ERROR;
}

//*******************ОБРАБОТКА КНОПОКИ СТОП*********************

int UNSFillingPointDevice::processStopButtonClick() {
	dbg("[STOPPED] Нажатие кнопки.");
	stop();
	return ERROR_CODE::NO_ERROR;
}

//*******************ОБРАБОТКА КНОПОКИ СТАРТ*********************

int UNSFillingPointDevice::processStartButtonClick() {
	if (getState() == DEVICE_STATE::READY) {
		return buttonError(ERROR_CODE::BAD_DV_STATE);
	}

	if (getState() == DEVICE_STATE::EQUIPPED) {
		dbg("[READY] Нажатие кнопки START.");
		buttonMessage(" GO ", button_message_display_time);
		setState(DEVICE_STATE::READY);
		fill_start = QTime::currentTime();
		return ERROR_CODE::NO_ERROR;
	}

	if (getState() == DEVICE_STATE::OVERWEIGHT) {
		if (!uns_unit->state_stable) {
			dbg(QString("Невозможно возобновить наполнеие: баллон нестабилен."));
			return buttonError(ERROR_CODE::UNSTABLE_VAL);
		}
		if ((uns_unit->mass - uns_unit->tare) < gas_weight) {
			setState(DEVICE_STATE::READY);
			dbg("[READY] Возобновление наполения по нажатию кнопки START.");
			return ERROR_CODE::NO_ERROR;
		}
		dbg(QString("Невозможно возобновить наполнеие: баллон переполнен."));
		return buttonError(ERROR_CODE::OUT_OF_RANGE);

		return ERROR_CODE::NO_ERROR;
	}

	dbg(QString("[STOPPED] Нельзя перейти к наполнению: текущее состояние поста [%1]").arg(getStateName(getState())));
	stop();
	return ERROR_CODE::BAD_DV_STATE;
}

//*******************ОБРАБОТКА КНОПОКИ ТАРА*********************

int UNSFillingPointDevice::processTareButtonClick() {
	if (getState() == DEVICE_STATE::EMPTY) {
		//Сброс тары возможен только при стабильно пустой (или содержащей стакан) платформе.
		if (!uns_unit->state_stable) {
			dbg(QString("Невозможно сбросить тару: баллон нестабилен."));
			return buttonError(ERROR_CODE::UNSTABLE_VAL);
		}

		if ((fabs(uns_unit->mass - rack_weight) < (zero_weight_delta)) || (fabs(uns_unit->mass) < (zero_weight_delta))) {
			uns_unit->setTareCurrent();
			uns_unit->displayWeight();
			dbg(QString("Отныне платформа считается пустой при абсолютном показании весов в %1 кг.").arg(uns_unit->tare));
			return ERROR_CODE::NO_ERROR;
		}

		dbg(QString("Невозможно сбросить тару: на весах не пусто."));
		return buttonError(ERROR_CODE::OUT_OF_RANGE);
	}

	if (getState() == DEVICE_STATE::MAINTENANCE) {
		//Сброс тары возможен:	ЛИБО при стабильной содержащей баллон правильной массы платформе
		//						ЛИБО при стабильной содержащей стакан платформе
		if (!uns_unit->state_stable) {
			dbg(QString("Невозможно сбросить тару: баллон нестабилен."));
			return buttonError(ERROR_CODE::UNSTABLE_VAL);
		}

		if ((fabs(uns_unit->mass - rack_weight) < (zero_weight_delta)) || (fabs(uns_unit->mass) < (zero_weight_delta))) {
			uns_unit->setTareCurrent();
			uns_unit->displayWeight();
			dbg(QString("Отныне платформа считается пустой при абсолютном показании весов в %1 кг.").arg(uns_unit->tare));
			return ERROR_CODE::NO_ERROR;
		}

		if (fabs(uns_unit->mass - uns_unit->tare - empty_tank_weight) < empty_tank_weight_delta) {
			fill_tare = (uns_unit->mass - uns_unit->tare); //Масса пустого баллона с накрученным вентилем
			uns_unit->setTareCurrent();
			setState(DEVICE_STATE::EQUIPPED);
			equip_timer->start();
			dbg("[EQUIPPED] Нажатие кнопки TARE.");
			return ERROR_CODE::NO_ERROR;
		}

		dbg(QString("Невозможно сбросить тару: Абсолютные показания весов: %1. Текущая тара: %2. Вес стойки: %3. Вес пустого баллона: %4±%5.").
			arg(uns_unit->mass).
			arg(uns_unit->tare).
			arg(rack_weight).
			arg(empty_tank_weight).
			arg(empty_tank_weight_delta)
		);
		return buttonError(ERROR_CODE::OUT_OF_RANGE);
	}

	if (getState() == DEVICE_STATE::OVERWEIGHT) {
		//Сброс тары возможен только при стабильно пустой (или содержащей стакан) платформе.
		//Это формальный конец процесса заполнения баллона
		if (!uns_unit->state_stable) {
			dbg(QString("Невозможно сбросить тару: показания весов нестабилены."));
			return buttonError(ERROR_CODE::UNSTABLE_VAL);
		}

		if ((fabs(uns_unit->mass - rack_weight) < (zero_weight_delta)) || (fabs(uns_unit->mass) < (zero_weight_delta))) {
			uns_unit->setTareCurrent();
			uns_unit->displayWeight();
			emit fillingComplete(this);
			setState(DEVICE_STATE::EMPTY);
			dbg("[EMPTY] Нажатие кнопки TARE. Переход из положения [OVERWEIGHT].");
			return ERROR_CODE::NO_ERROR;
		}

		dbg(QString("Невозможно сбросить тару: на весах не пусто."));
		return buttonError(ERROR_CODE::OUT_OF_RANGE);
	}

	if (getState() == DEVICE_STATE::STOPPED) {
		//Сброс тары возможен только при стабильно пустой (или содержащей стакан) платформе.
		if (!uns_unit->state_stable) {
			dbg(QString("Невозможно сбросить тару: показания весов нестабилены."));
			return buttonError(ERROR_CODE::UNSTABLE_VAL);
		}

		if ((fabs(uns_unit->mass - rack_weight) < (zero_weight_delta)) || (fabs(uns_unit->mass) < (zero_weight_delta))) {
			uns_unit->setTareCurrent();
			uns_unit->displayWeight();
			dbg(QString("Отныне платформа считается пустой при абсолютном показании весов в %1 кг.").arg(uns_unit->tare));
			setState(DEVICE_STATE::EMPTY);
			dbg("[EMPTY] Нажатие кнопки TARE. Переход из положения [STOPPED].");
			return ERROR_CODE::NO_ERROR;
		}

		dbg(QString("Невозможно сбросить тару: на весах не пусто."));
		return ERROR_CODE::OUT_OF_RANGE; //Не показываем button_message, ведь всё ещё STOP
	}

	dbg(QString("Нельзя сбросить тару: текущее состояние поста [%1]").arg(getStateName(getState())));
	return buttonError(ERROR_CODE::BAD_DV_STATE);
}

//*******************ОБРАБОТКА ИЗМЕНЕНИЙ ВЕСА*********************

int UNSFillingPointDevice::processUnitData() {
	if (((getState() == DEVICE_STATE::UNKNOWN) || (getState() == DEVICE_STATE::MAINTENANCE))) {
		if ((fabs(uns_unit->mass - uns_unit->tare) <= zero_weight_delta) && (uns_unit->state_stable)) {
			setState(DEVICE_STATE::EMPTY); //На весах пусто, или стоит стакан (если предусмотрен текущей модой). Показания стабильны.
			dbg(QString("[EMPTY] Автоматический переход из состояния [%1]. На весах %2 кг. Значение %3.").arg(getStateName(getPreviousState())).arg(uns_unit->mass).arg(uns_unit->value));
		}
	}

	if (((getState() == DEVICE_STATE::UNKNOWN) || (getState() == DEVICE_STATE::EMPTY))) {
		if (((fabs(uns_unit->mass - uns_unit->tare) > zero_weight_delta) || (!uns_unit->state_stable)) && (uns_unit->value != 0)) { //(uns_unit->value != 0) <- для того, чтобы не попадать в MAINTENANCE с иницации uns_unit
			setState(DEVICE_STATE::MAINTENANCE); //Идет установка баллона, присоединение шлангов и т.д.
			dbg(QString("[MAINTENANCE] Автоматический переход из состояния [%1]. На весах %2 кг. Значение %3.").arg(getStateName(getPreviousState())).arg(uns_unit->mass).arg(uns_unit->value));
		}
	}

	if (getState() == DEVICE_STATE::READY) { // Готов к наполнению. Возможно оно даже идет!
		if ((uns_unit->mass - uns_unit->tare) >= gas_weight) {
			if (overweight_suss) { //Два раза подряд перевес! Пора вырубаться.
				overweight_suss = false;
				fill_end   = QTime::currentTime();
				fill_mass  = uns_unit->mass - uns_unit->tare; //Масса закачанного газа
				fill_mode  = tank_type;
				setState(DEVICE_STATE::OVERWEIGHT); // Прервать наполнение!
				dbg(QString("[OVERWEIGHT] Автоматический переход из состояния READY. На весах %1 кг [%2].").arg(uns_unit->mass).arg(uns_unit->value));
			}
			else overweight_suss = true;
		}
		else overweight_suss = false;
	}

	return ERROR_CODE::NO_ERROR;
}

//*******************ОБРАБОТКА РАЗРЫВА СОЕДИНЕНИЯ*********************

int UNSFillingPointDevice::processUnitStateChange() {
	int s = uns_unit->getState();
	int p = uns_unit->getPreviousState();

	if (s == DEVICE_STATE::HANGED) {
		setState(DEVICE_STATE::HANGED); // Прервать наполнение!
		dbg("[HANGED] Потеря связи с весами");
		return ERROR_CODE::NO_ERROR;
	}
	if ((s == DEVICE_STATE::ON) && (p == DEVICE_STATE::HANGED)) { 
		setState(getPreviousState()); //Вернуться к предыдущему состоянию
		dbg(QString("[%1] Восстановлена связь с весами.").arg(getStateName(getState())));
		if (s == DEVICE_STATE::STOPPED)
			stop();
		return ERROR_CODE::NO_ERROR;
	}

	return ERROR_CODE::NO_ERROR;
}

//*******************ОБРАБОТКА ПРЕДОХРАНИТЕЛЯ НАПОЛНЕНИЯ*********************

int UNSFillingPointDevice::processEquipTimerTimeout() {
	if (getState() == DEVICE_STATE::HANGED) {
		dbg(QString("Не нажата кнопка [START] в течение %1[ms]. Кроме того, таймаут выпал на состояние HANGED. Меняем previous_state.").arg(equip_timer_interval));
		previous_state = DEVICE_STATE::STOPPED;
		return ERROR_CODE::NO_ERROR;
	}
	if (getState() == DEVICE_STATE::EQUIPPED) {
		dbg(QString("[STOPPED] Не нажата кнопка [START] в течение %1[ms]").arg(equip_timer_interval));
		stop();
		return ERROR_CODE::NO_ERROR;
	}
	return ERROR_CODE::NO_ERROR;
}