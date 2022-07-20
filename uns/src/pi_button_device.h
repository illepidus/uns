#pragma once
#include "global.h"
#include "raspberry_gpio.h"
#include "abstract_button_device.h"

class PiButtonDevice : public AbstractButtonDevice
{
	Q_OBJECT

	public:
		PiButtonDevice(QString);
	private slots:
		int handleInterrupt();
	private slots:
		int hardwareInit();
		int hardwareDeinit();
	private:
		int pin;
		bool pull_up;
		QFileSystemWatcher watcher;
};