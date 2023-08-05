#pragma once
#include "global.h"
#include "abstract_device.h"

class AbstractPowerDevice : public AbstractDevice
{
	Q_OBJECT

	public:
		AbstractPowerDevice(QString);
	public slots:
		int start();
		int stop();
		int switchOn();
		int switchOff();
		int switchState();
		int delayedOn();
		int delayedOn(int);
		int delayedOff();
		int delayedOff(int);
		int delayedSwitch();
		int delayedSwitch(int);
	protected slots:
		virtual int hardwareDeinit();
		virtual int hardwareInit();
		virtual int hardwareSwitchOn();
		virtual int hardwareSwitchOff();
	protected:
		int default_delay;
		int initial_state;
		QTimer *onTimer;
		QTimer *offTimer;
		QTimer *switchTimer;
};