#include "pi_button_device.h"

PiButtonDevice::PiButtonDevice(QString n) : AbstractButtonDevice(n)
{
}

int PiButtonDevice::hardwareInit() {
	pin = PI_GPIO_PIN_MIN - 1;
	pull_up = true;

	if (getSetting("pin").isValid())
		pin = getSetting("pin").toInt();
	if (getSetting("pull_up").isValid())
		pull_up = getSetting("pull_up").toBool();

	if ((pin < PI_GPIO_PIN_MIN) || (pin >= PI_GPIO_PIN_MAX))
		return ERROR_CODE::UNEXPD_ENTRY;

	int e = ERROR_CODE::NO_ERROR;
	e = RaspberryGPIO::initPin(pin); if (e != ERROR_CODE::NO_ERROR) return e; 
	e = RaspberryGPIO::setPinInput(pin); if (e != ERROR_CODE::NO_ERROR) return e; 
	e = RaspberryGPIO::setPinMode(pin, pull_up ? PI_GPIO_MODE_UP : PI_GPIO_MODE_DOWN); if (e != ERROR_CODE::NO_ERROR) return e; 
	e = RaspberryGPIO::setPinInterruptAll(pin); if (e != ERROR_CODE::NO_ERROR) return e; 

	watcher.addPath(QString(PI_GPIO_TEMPLATE_VALUE).arg(pin));
	QObject::connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(handleInterrupt()));

	return ERROR_CODE::NO_ERROR;
}

int PiButtonDevice::hardwareDeinit() {
	return RaspberryGPIO::deinitPin(pin);
}

int PiButtonDevice::handleInterrupt()
{
	int v;
	int e = RaspberryGPIO::getPinValue(pin, v); if (e != ERROR_CODE::NO_ERROR) return e; 

	if (v == (pull_up ? 0 : 1)) {
		down();
		return ERROR_CODE::NO_ERROR;
	}

	up();
	return ERROR_CODE::NO_ERROR;

}