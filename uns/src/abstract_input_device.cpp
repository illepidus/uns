#include "abstract_input_device.h"

AbstractInputDevice::AbstractInputDevice(QString n) : AbstractDevice(n)
{
	
}

int AbstractInputDevice::hardwareInit() {
	//MODIFY THIS FUNCTION IN SUBCLASS (CONNECT HARDWARE HERE)
	return ERROR_CODE::NO_ERROR;
}

int AbstractInputDevice::start()
{
	setState(DEVICE_STATE::ON);
	if (hardwareInit() != ERROR_CODE::NO_ERROR) {
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::GENERL_ERROR;
	}
	return ERROR_CODE::NO_ERROR;
}

int AbstractInputDevice::stop()
{
	setState(DEVICE_STATE::OFF);
	return ERROR_CODE::NO_ERROR;
}