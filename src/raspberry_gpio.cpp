#include "raspberry_gpio.h"

void RaspberryGPIO::detonate()
{
	qDebug() << "RaspberryGPIO::detonate()";
}

bool RaspberryGPIO::checkPinRange(int pin)
{
	if (pin > PI_GPIO_PIN_MAX) return false;
	if (pin < PI_GPIO_PIN_MIN) return false;
	return true;
}


int RaspberryGPIO::initPin(int pin)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QFile file(PI_GPIO_PATH_EXPORT);
	if (!file.open(QIODevice::WriteOnly)) return ERROR_CODE::HARDWR_ERROR;
	QTextStream stream(&file);
	stream << QString::number(pin);
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::deinitPin(int pin)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QFile file(PI_GPIO_PATH_UNEXPORT);
	if (!file.open(QIODevice::WriteOnly)) return ERROR_CODE::HARDWR_ERROR;
	QTextStream stream(&file);
	stream << QString::number(pin);
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinDirection(int pin, int d)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;
	QString dir = (d==PI_GPIO_INPUT) ? "in" : "out";

	QFile file(QString(PI_GPIO_TEMPLATE_DIRECTION).arg(pin));
	if (!file.open(QIODevice::WriteOnly)) return ERROR_CODE::HARDWR_ERROR;
	QTextStream stream(&file);
	stream << dir;
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinInput(int pin)
{
	return setPinDirection(pin, PI_GPIO_INPUT);
}

int RaspberryGPIO::setPinOutput(int pin)
{
	return setPinDirection(pin, PI_GPIO_OUTPUT);
}

int RaspberryGPIO::setPinInterrupt(int pin, int i)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QString interrupt = (i==PI_GPIO_EDGE_BOTH) ? "both" : (i==PI_GPIO_EDGE_FALLING) ? "falling" : (i==PI_GPIO_EDGE_RISING) ? "rising" : "none";

	QFile file(QString(PI_GPIO_TEMPLATE_EDGE).arg(pin));
	if (!file.open(QIODevice::WriteOnly)) return ERROR_CODE::HARDWR_ERROR;
	QTextStream stream(&file);
	stream << interrupt;
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinInterruptRising(int pin)
{
	return setPinInterrupt(pin, PI_GPIO_EDGE_RISING);
}

int RaspberryGPIO::setPinInterruptFalling(int pin)
{
	return setPinInterrupt(pin, PI_GPIO_EDGE_FALLING);
}

int RaspberryGPIO::setPinInterruptAll(int pin)
{
	return setPinInterrupt(pin, PI_GPIO_EDGE_BOTH);
}

int RaspberryGPIO::setPinInterruptNone(int pin)
{
	return setPinInterrupt(pin, PI_GPIO_EDGE_NONE);
}

int RaspberryGPIO::getPinValue(int pin, int &v)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QFile file(QString(PI_GPIO_TEMPLATE_VALUE).arg(pin));
	if (!file.open(QIODevice::ReadOnly)) 
		return ERROR_CODE::HARDWR_ERROR;

	QString val;
	QTextStream stream(&file);
	stream >> val;
	v = val.toInt();
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinValue(int pin, int v)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QString val = (v==0) ? "0" : "1";

	QFile file(QString(PI_GPIO_TEMPLATE_VALUE).arg(pin));
	if (!file.open(QIODevice::WriteOnly)) 
		return ERROR_CODE::HARDWR_ERROR;

	QTextStream stream(&file);
	stream << val;
	file.close();
	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinOn(int pin)
{
	return setPinValue(pin, 1);
}

int RaspberryGPIO::setPinOff(int pin)
{
	return setPinValue(pin, 0);
}

int RaspberryGPIO::setPinMode(int pin, int m)
{
	if (!checkPinRange(pin))
		return ERROR_CODE::UNEXPD_ENTRY;

	QString mode = (m==PI_GPIO_MODE_DOWN) ? "down" : (m==PI_GPIO_MODE_UP) ? "up" : "tri";
	system((QString(PI_GPIO_TEMPLATE_MODE).arg(pin).arg(mode)).toLatin1());

	return ERROR_CODE::NO_ERROR;
}

int RaspberryGPIO::setPinModeUp(int pin)
{
	return setPinMode(pin, PI_GPIO_MODE_UP);
}

int RaspberryGPIO::setPinModeDown(int pin)
{
	return setPinMode(pin, PI_GPIO_MODE_DOWN);
}

int RaspberryGPIO::setPinModeTri(int pin)
{
	return setPinMode(pin, PI_GPIO_MODE_TRI);
}