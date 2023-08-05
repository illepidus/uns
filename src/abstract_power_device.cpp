#include "abstract_power_device.h"

AbstractPowerDevice::AbstractPowerDevice(QString n) : AbstractDevice(n)
{
	onTimer = new QTimer(this);
	offTimer = new QTimer(this);
	switchTimer = new QTimer(this);
	onTimer->setSingleShot(true);
	offTimer->setSingleShot(true);
	switchTimer->setSingleShot(true);

	connect(onTimer, SIGNAL(timeout()), this, SLOT(switchOn()));
	connect(offTimer, SIGNAL(timeout()), this, SLOT(switchOff()));
	connect(switchTimer, SIGNAL(timeout()), this, SLOT(switchState()));
}

int AbstractPowerDevice::hardwareSwitchOn() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	dbg("SWITCHED ON");
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::hardwareSwitchOff() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	dbg("SWITCHED OFF");
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::hardwareInit() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::hardwareDeinit() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::start()
{
	if (hardwareInit() != ERROR_CODE::NO_ERROR) {
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::GENERL_ERROR;
	}

	default_delay = 1000;
	if (getSetting("default_delay").isValid())
		default_delay = getSetting("default_delay").toInt();

	initial_state = DEVICE_STATE::OFF;
	if (getSetting("initial_state").isValid())
		initial_state = getSetting("initial_state").toInt();

	if (initial_state == DEVICE_STATE::ON)
		switchOn();
	else
		switchOff();

	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::stop()
{
	return switchOff();
}

int AbstractPowerDevice::switchOn() {
	if (state == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;
	if (state==DEVICE_STATE::ON)
		return ERROR_CODE::NO_ERROR;
	if (hardwareSwitchOn() != ERROR_CODE::NO_ERROR)
		return ERROR_CODE::GENERL_ERROR;

	setState(DEVICE_STATE::ON);
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::switchOff() {
	if (state == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;
	if (state==DEVICE_STATE::OFF)
		return ERROR_CODE::NO_ERROR;
	if (hardwareSwitchOff() != ERROR_CODE::NO_ERROR)
		return ERROR_CODE::GENERL_ERROR;

	setState(DEVICE_STATE::OFF);

	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::switchState() {
	if (state == DEVICE_STATE::ON)
		return switchOff();
	if (state == DEVICE_STATE::OFF)
		return switchOn();
	return ERROR_CODE::BAD_DV_STATE;
}

int AbstractPowerDevice::delayedOn(int ms) {
	onTimer->start(ms);
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::delayedOff(int ms) {
	offTimer->start(ms);
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::delayedSwitch(int ms) {
	switchTimer->start(ms);
	return ERROR_CODE::NO_ERROR;
}

int AbstractPowerDevice::delayedOn() {
	return delayedOn(default_delay);
}

int AbstractPowerDevice::delayedOff() {
	return delayedOff(default_delay);
}

int AbstractPowerDevice::delayedSwitch() {
	return delayedSwitch(default_delay);
}