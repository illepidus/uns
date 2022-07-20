#include "abstract_button_device.h"

AbstractButtonDevice::AbstractButtonDevice(QString n) : AbstractDevice(n)
{
	click_timer = new QTime();
	double_click_timer = new QTimer();
}

int AbstractButtonDevice::hardwareInit() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::hardwareDeinit() {
	//MODIFY THIS FUNCTION IN SUBCLASS
	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::start()
{
	if (hardwareInit() != ERROR_CODE::NO_ERROR) {
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::GENERL_ERROR;
	}

	click_interval = 30;
	long_click_interval = 500;
	double_click_interval = 200;

	if (getSetting("click_interval").isValid())
		click_interval = getSetting("click_interval").toInt();
	if (getSetting("long_click_interval").isValid())
		long_click_interval = getSetting("long_click_interval").toInt();
	if (getSetting("double_click_interval").isValid())
		double_click_interval = getSetting("double_click_interval").toInt();

	first_click = true;
	double_click_timer->setSingleShot(true);
	double_click_timer->setInterval(double_click_interval);
	QObject::connect(double_click_timer, SIGNAL(timeout()), this, SLOT(click()));

	setState(DEVICE_STATE::ON);
	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::stop()
{
	if (hardwareDeinit() != ERROR_CODE::NO_ERROR) {
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::GENERL_ERROR;
	}
	setState(DEVICE_STATE::OFF);
	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::down()
{
	//CONNECT THIS TO HARDWARE
	if (getState() == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;

	if (getState() == DEVICE_STATE::DOWN) //ошибочный вызов
		return ERROR_CODE::NO_ERROR;

	click_timer->restart();
	setState(DEVICE_STATE::DOWN);
	emit pressed();

	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::up()
{
	//CONNECT THIS TO HARDWARE
	if (getState() == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;

	if (getState() == DEVICE_STATE::UP) //ошибочный вызов
		return ERROR_CODE::NO_ERROR;

	setState(DEVICE_STATE::UP);

	if (click_timer->elapsed() < click_interval) {
		//дребезг контакта
		return ERROR_CODE::NO_ERROR;
	}
	emit released();

	if ((click_timer->elapsed() >= click_interval) && (click_timer->elapsed() < long_click_interval)) {
		if (first_click) {
			first_click = false;
			double_click_timer->start();
		}
		else {
			doubleClick();
		}
		//return click();
	}

	if (click_timer->elapsed() >= long_click_interval) {
		return longClick();
	}

	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::click()
{
	//CAN BE CALLED FROM OUTSIDE
	first_click = true;
	if (getState() == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;

	emit clicked();
	dbg("SINGLE CLICKED");
	return ERROR_CODE::NO_ERROR;
}


int AbstractButtonDevice::longClick()
{
	//CAN BE CALLED FROM OUTSIDE
	if (getState() == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;

	first_click = true;
	emit longClicked();
	dbg("LONG CLICKED");
	return ERROR_CODE::NO_ERROR;
}

int AbstractButtonDevice::doubleClick()
{
	//CAN BE CALLED FROM OUTSIDE
	if (getState() == DEVICE_STATE::ERROR) 
		return ERROR_CODE::BAD_DV_STATE;

	double_click_timer->stop();
	first_click = true;
	emit doubleClicked();
	dbg("DOUBLE CLICKED");
	return ERROR_CODE::NO_ERROR;
}