#pragma once
#include "global.h"
#include "raspberry_gpio.h"
#include "abstract_power_device.h"

class PiPowerDevice : public AbstractPowerDevice
{
	Q_OBJECT

	public:
		PiPowerDevice(QString);
	private slots:
		int hardwareInit();
		int hardwareDeinit();
		int hardwareSwitchOn();
		int hardwareSwitchOff();
	private:
		int pin;
};