#include "abstract_modbus_device.h"

AbstractModbusDevice::AbstractModbusDevice(QString n, ModbusDevice *b) : AbstractDevice(n)
{
	modbus = b;
	hang_timer = new QTimer(this);
	hang_timer->setSingleShot(false);

	QObject::connect(modbus, SIGNAL(dataUnitRecieved(QByteArray, QByteArray)), this, SLOT(processDataUnit(QByteArray, QByteArray)));
	QObject::connect(hang_timer, SIGNAL(timeout()), this, SLOT(hangUp()));
}

int AbstractModbusDevice::hardwareInit() {
	return ERROR_CODE::NO_ERROR;
}

int AbstractModbusDevice::request(QByteArray du) {
	if ((getState() == DEVICE_STATE::ERROR) || (getState() == DEVICE_STATE::DISABLED))
		return ERROR_CODE::BAD_DV_STATE;
	return modbus->sendDataUnit(du);
}

int AbstractModbusDevice::request(int fn, QByteArray du) {
	if ((getState() == DEVICE_STATE::ERROR) || (getState() == DEVICE_STATE::DISABLED))
		return ERROR_CODE::BAD_DV_STATE;
	if ((fn < 0) || (fn > MAX_MODBUS_FUNCTION_CODE))
		return ERROR_CODE::UNEXPD_ENTRY;
	return modbus->sendDataUnit(ModbusDevice::dataUnit(modbus_id, fn, du));
}

int AbstractModbusDevice::addRoutine(QByteArray du) {
	if ((getState() == DEVICE_STATE::ERROR) || (getState() == DEVICE_STATE::DISABLED))
		return ERROR_CODE::BAD_DV_STATE;
	return modbus->addRoutine(du);
}


int AbstractModbusDevice::addRoutine(int fn, QByteArray du) {
	if ((getState() == DEVICE_STATE::ERROR) || (getState() == DEVICE_STATE::DISABLED))
		return ERROR_CODE::BAD_DV_STATE;
	if ((fn < 0) || (fn > MAX_MODBUS_FUNCTION_CODE))
		return ERROR_CODE::UNEXPD_ENTRY;
	return modbus->addRoutine(ModbusDevice::dataUnit(modbus_id, fn, du));
}



int AbstractModbusDevice::hardwareProcessDataUnit(QByteArray du, QByteArray rdu) {
	dbg(QString("GOT DATA UNIT [%1] AS A RESPONSE FOR [%2]").arg(ModbusDevice::toHumanReadable(du)).arg(ModbusDevice::toHumanReadable(rdu)));
	return ERROR_CODE::NO_ERROR;
}


int AbstractModbusDevice::start()
{
	modbus_id = 0;
	hang_interval = 1000;
	disabled = false;

	if (getSetting("disabled").isValid())
		disabled = getSetting("disabled").toBool();

	if (disabled == true) {
		setState(DEVICE_STATE::DISABLED);
		return ERROR_CODE::NO_ERROR;
	}

	if (getSetting("modbus_id").isValid()) {
		modbus_id = getSetting("modbus_id").toInt();
	}
	else {
		dbg("DEVICE MODBUS ID NOT SPECIFIED");
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::MISING_VARBL;
	}

	if (getSetting("hang_interval").isValid())
		hang_interval = getSetting("hang_interval").toInt();

	if (modbus == NULL) {
		dbg("modbus POINTS NOWHERE");
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::NULL_POINTER;
	}

	if (hardwareInit() != ERROR_CODE::NO_ERROR) {
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::GENERL_ERROR;
	}

	setState(DEVICE_STATE::OFF);

	return ERROR_CODE::NO_ERROR;
}

int AbstractModbusDevice::stop()
{
	setState(DEVICE_STATE::OFF);
	return ERROR_CODE::NO_ERROR;
}

int AbstractModbusDevice::processDataUnit(QByteArray du, QByteArray rdu)
{
	if (ModbusDevice::getID(du) != modbus_id)
		return ERROR_CODE::UNEXPD_ENTRY;

	int r = hardwareProcessDataUnit(du, rdu);

	if (r == ERROR_CODE::NO_ERROR) {
		if (hang_interval > 0) 
			//dbg(QString("%1 ms to HANGED").arg(hang_timer->remainingTime()));
			hang_timer->start(hang_interval);

		if (getState() != DEVICE_STATE::ON) {
			//dbg("Device is ON now");
			setState(DEVICE_STATE::ON);
		}
		emit newData();
	}
	return r;
}

int AbstractModbusDevice::hangUp() {
	if ((getState() == DEVICE_STATE::ERROR) || (getState() == DEVICE_STATE::DISABLED))
		return ERROR_CODE::BAD_DV_STATE;
	if (getState() == DEVICE_STATE::HANGED)
		return ERROR_CODE::NO_ERROR;;

	//dbg("Device hanged up :/");
	//dbg(modbus->currentStatus());
	setState(DEVICE_STATE::HANGED);
	return ERROR_CODE::NO_ERROR;
}