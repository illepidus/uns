#include "pi_matrix_keyboard_device.h"

PiMatrixKeyboardDevice::PiMatrixKeyboardDevice(QString n) : AbstractInputDevice(n)
{
	
}

int PiMatrixKeyboardDevice::hardwareInit() {
	int e = ERROR_CODE::NO_ERROR;
	current_row = 0;

	pull_up = true;
	if (getSetting("pull_up").isValid())
		pull_up = getSetting("pull_up").toBool();

	bounce_time = 10;
	if (getSetting("bounce_time").isValid())
		bounce_time = getSetting("bounce_time").toInt();

	num_columns = 4;
	if (getSetting("num_columns").isValid())
		num_columns = getSetting("num_columns").toInt();

	num_rows = 4;
	if (getSetting("num_rows").isValid())
		num_rows = getSetting("num_rows").toInt();

	bounce_timer.setInterval(bounce_time);
	bounce_timer.setSingleShot(true);

	in_pins.clear();
	for (int i=0; i<num_rows; i++) {
		int in_pin = PI_GPIO_PIN_INVALID;
		if (getSetting(QString("in_pin_%1").arg(i)).isValid())
			in_pin = getSetting(QString("in_pin_%1").arg(i)).toInt();
		if (!RaspberryGPIO::checkPinRange(in_pin))
			return ERROR_CODE::UNEXPD_ENTRY;
		in_pins << in_pin;
	}

	out_pins.clear();
	for (int i=0; i<num_columns; i++) {
		int out_pin = PI_GPIO_PIN_INVALID;
		if (getSetting(QString("out_pin_%1").arg(i)).isValid())
			out_pin = getSetting(QString("out_pin_%1").arg(i)).toInt();
		if (!RaspberryGPIO::checkPinRange(out_pin))
			return ERROR_CODE::UNEXPD_ENTRY;
		out_pins << out_pin;
	}

	key_codes.clear();
	for (int i=0; i<num_rows*num_columns; i++) {
		QChar code = STANDART_KEYCODE_TABLE[i];
		if (getSetting(QString("charcode%1").arg(i)).isValid())
			code = getSetting(QString("charcode%1").arg(i)).toString()[0];
		key_codes << code;
	}

	command_terminator = 14;
	if (getSetting("command_terminator").isValid())
		command_terminator = getSetting("command_terminator").toInt();
	if (command_terminator >= num_rows*num_columns)
			return ERROR_CODE::UNEXPD_ENTRY;
	command_terminator_char = key_codes.at(command_terminator);

	command_cancel = 12;
	if (getSetting("command_cancel").isValid())
		command_cancel = getSetting("command_cancel").toInt();
	if (command_cancel >= num_rows*num_columns)
			return ERROR_CODE::UNEXPD_ENTRY;
	command_cancel_char = key_codes.at(command_cancel);

	max_command_size = 16;
	if (getSetting("max_command_size").isValid())
		max_command_size = getSetting("max_command_size").toInt();

	for (int i=0; i<num_rows; i++) {
		e = RaspberryGPIO::initPin(in_pins.at(i)); if (e != ERROR_CODE::NO_ERROR) return e; 
		e = RaspberryGPIO::setPinInput(in_pins.at(i)); if (e != ERROR_CODE::NO_ERROR) return e; 
		e = RaspberryGPIO::setPinMode(in_pins.at(i), pull_up ? PI_GPIO_MODE_UP : PI_GPIO_MODE_DOWN); if (e != ERROR_CODE::NO_ERROR) return e; 
		e = RaspberryGPIO::setPinInterrupt(in_pins.at(i), pull_up ? PI_GPIO_EDGE_FALLING : PI_GPIO_EDGE_RISING); if (e != ERROR_CODE::NO_ERROR) return e; 
		watcher.addPath(QString(PI_GPIO_TEMPLATE_VALUE).arg(in_pins.at(i)));
	}
	for (int i=0; i<num_columns; i++) {
		e = RaspberryGPIO::initPin(out_pins.at(i)); if (e != ERROR_CODE::NO_ERROR) return e; 
		e = RaspberryGPIO::setPinOutput(out_pins.at(i)); if (e != ERROR_CODE::NO_ERROR) return e; 
		e = RaspberryGPIO::setPinValue(out_pins.at(i), (pull_up ? 0 : 1)); if (e != ERROR_CODE::NO_ERROR) return e;
	}

	QObject::connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(handleInterrupt(QString)));
	QObject::connect(&bounce_timer, SIGNAL(timeout()), this, SLOT(processButtonPress()));

	return ERROR_CODE::NO_ERROR;
}

int PiMatrixKeyboardDevice::hardwareDeinit() {
	for (int i=0; i<num_rows; i++) 
		RaspberryGPIO::deinitPin(in_pins.at(i));

	for (int i=0; i<num_columns; i++) 
		RaspberryGPIO::deinitPin(out_pins.at(i));

	return ERROR_CODE::NO_ERROR;
}


int PiMatrixKeyboardDevice::handleInterrupt(QString s)
{
	QRegExp rx(PI_GPIO_PATTERN_VALUE);
	int pos = rx.indexIn(s);
	if (pos < 0)
		return ERROR_CODE::UNEXPD_ENTRY;

	int pin = rx.cap(1).toInt();
	int row = in_pins.indexOf(pin);
	if (row < 0)
		return ERROR_CODE::UNEXPD_ENTRY;

	int val = 0;
	int e = RaspberryGPIO::getPinValue(pin, val); if (e != ERROR_CODE::NO_ERROR) return e;

	if (((val == 1) && pull_up) || ((val == 0) && (!pull_up)))
		return ERROR_CODE::NO_ERROR; //Дребезг

	current_row = row;
	bounce_timer.start();

	return ERROR_CODE::NO_ERROR;
}

int PiMatrixKeyboardDevice::processButtonPress()
{
	int current_column = 0;
	int val = 0; int e = RaspberryGPIO::getPinValue(in_pins.at(current_row), val); if (e != ERROR_CODE::NO_ERROR) return e;

	if (((val == 1) && pull_up) || ((val == 0) && (!pull_up)))
		return ERROR_CODE::NO_ERROR; //Дребезг

	for (int i=0;i<num_rows;i++) //Прекращаем следить за прерываениями
		e = RaspberryGPIO::setPinInterruptNone(in_pins.at(i)); if (e != ERROR_CODE::NO_ERROR) return e; 

	int err = ERROR_CODE::NO_ERROR;

	for (int i=0; i<num_columns; i++) { //УБИРАЕМ РАЗНОСТЬ ПОТЕНЦИАЛОВ СО ВСЕХ КОЛОНОК
		int e = RaspberryGPIO::setPinValue(out_pins.at(i), (pull_up ? 1 : 0)); if (e != ERROR_CODE::NO_ERROR) err = e;
	}

	for (int i=0; i<num_columns; i++) {
		int e = RaspberryGPIO::setPinValue(out_pins.at(i), (pull_up ? 0 : 1)); if (e != ERROR_CODE::NO_ERROR) err = e;

		int val = 0; e = RaspberryGPIO::getPinValue(in_pins.at(current_row), val); if (e != ERROR_CODE::NO_ERROR) err = e;
		if (((val == 0) && pull_up) || ((val == 1) && (!pull_up))) {
			current_column = i;
		}

		e = RaspberryGPIO::setPinValue(out_pins.at(i), (pull_up ? 1 : 0)); if (e != ERROR_CODE::NO_ERROR) err = e;
	}

	for (int i=0; i<num_columns; i++) { //ВОЗВРАЩАЕМ РАЗНОСТЬ ПОТЕНЦИАЛОВ НА ВСЕ КОЛОНКИ
		int e = RaspberryGPIO::setPinValue(out_pins.at(i), (pull_up ? 0 : 1)); if (e != ERROR_CODE::NO_ERROR) err = e;
	}

	for (int i=0;i<num_rows;i++)  //Продолжаем следить за прерываениями
		e = RaspberryGPIO::setPinInterrupt(in_pins.at(i), pull_up ? PI_GPIO_EDGE_FALLING : PI_GPIO_EDGE_RISING); if (e != ERROR_CODE::NO_ERROR) err = e; 

	if (err != ERROR_CODE::NO_ERROR)
		return err;

	pressButton(current_row * num_columns + current_column);
	return ERROR_CODE::NO_ERROR;
}

int PiMatrixKeyboardDevice::pressButton(int b) {
	emit newData(b);
	emit newData(key_codes.at(b));

	if (b == command_cancel) {
		command_line.clear();
		emit cancel();
	}
	else if (b == command_terminator) {
		dbg(QString("CMD: ") + command_line);
		emit newData(command_line);
		emit command(command_line);
		command_line.clear();
	}
	else {
		if (command_line.size() < max_command_size)
			command_line += key_codes.at(b);
		else
			command_line = command_line.right(max_command_size - 1) + key_codes.at(b);

		emit keyPress(key_codes.at(b), command_line);
	}

	return ERROR_CODE::NO_ERROR;
}