#include "pi_power_device.h"

PiPowerDevice::PiPowerDevice(QString n) : AbstractPowerDevice(n)
{
	
}


int PiPowerDevice::hardwareInit() {
	pin = PI_GPIO_PIN_MIN - 1;
	if (getSetting("pin").isValid())
		pin = getSetting("pin").toInt();

	if ((pin < PI_GPIO_PIN_MIN) || (pin > PI_GPIO_PIN_MAX))
		return ERROR_CODE::UNEXPD_ENTRY;

	int e = ERROR_CODE::NO_ERROR;
	e = RaspberryGPIO::initPin(pin);if (e != ERROR_CODE::NO_ERROR) return e;
	e = RaspberryGPIO::setPinOutput(pin); if (e != ERROR_CODE::NO_ERROR) return e;

	return ERROR_CODE::NO_ERROR;
}

int PiPowerDevice::hardwareDeinit() {

	return RaspberryGPIO::deinitPin(pin);
}

int PiPowerDevice::hardwareSwitchOn() {
	return RaspberryGPIO::setPinOn(pin);
}

int PiPowerDevice::hardwareSwitchOff() {
	return RaspberryGPIO::setPinOff(pin);
}