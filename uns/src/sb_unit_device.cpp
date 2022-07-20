#include "sb_unit_device.h"

SBUnitDevice::SBUnitDevice(QString n, ModbusDevice *b) : AbstractModbusDevice(n, b)
{
	
}


int SBUnitDevice::detonate()
{

	return ERROR_CODE::NO_ERROR;
}


int SBUnitDevice::hardwareInit()
{
	routine_request_du.append((char) 0x00);
	routine_request_du.append((char) 0xD4);
	routine_request_du.append((char) 0x00);
	routine_request_du.append((char) 0x02);
	routine_request_du = ModbusDevice::dataUnit(modbus_id, 0x03, routine_request_du);
	addRoutine(routine_request_du);

	initial_request_du.append((char) 0x00);
	initial_request_du.append((char) 0x1C);
	initial_request_du.append((char) 0x00);
	initial_request_du.append((char) 0x1B);
	initial_request_du = ModbusDevice::dataUnit(modbus_id, 0x03, initial_request_du);
	request(initial_request_du);

	return ERROR_CODE::NO_ERROR;
}


int SBUnitDevice::hardwareProcessDataUnit(QByteArray du, QByteArray rdu)
{
	int fn = ModbusDevice::getFnCode(du);
	if (rdu == initial_request_du) {
		if (fn != 0x03)
			return ERROR_CODE::HARDWR_ERROR;

		if (du.size() < 59) {
			dbg("REREQUESTING INITIALIZATION DATA");
			request(initial_request_du);
			return ERROR_CODE::UNEXPD_ENTRY;
		}
		device_mode        = ModbusDevice::decodeUInt16(du.mid( 3, 2)); setData("device_mode"       , device_mode       );
		update_interval    = ModbusDevice::decodeUInt16(du.mid( 5, 2)); setData("update_interval"   , update_interval   );
		hits_per_weight    = ModbusDevice::decodeUInt16(du.mid( 7, 2)); setData("hits_per_weight"   , hits_per_weight   );
		tare               = ModbusDevice::decodeFloat (du.mid( 9, 4)); setData("tare"              , tare              );
		scales1_intercept  = ModbusDevice::decodeFloat (du.mid(13, 4)); setData("scales1_intercept" , scales1_intercept );
		scales1_slope      = ModbusDevice::decodeFloat (du.mid(17, 4)); setData("scales1_slope"     , scales1_slope     );
		scales2_intercept  = ModbusDevice::decodeFloat (du.mid(21, 4)); setData("scales2_intercept" , scales2_intercept );
		scales2_slope      = ModbusDevice::decodeFloat (du.mid(25, 4)); setData("scales2_slope"     , scales2_slope     );
		scales3_intercept  = ModbusDevice::decodeFloat (du.mid(29, 4)); setData("scales3_intercept" , scales3_intercept );
		scales3_slope      = ModbusDevice::decodeFloat (du.mid(33, 4)); setData("scales3_slope"     , scales3_slope     );
		scales4_intercept  = ModbusDevice::decodeFloat (du.mid(37, 4)); setData("scales4_intercept" , scales4_intercept );
		scales4_slope      = ModbusDevice::decodeFloat (du.mid(41, 4)); setData("scales4_slope"     , scales4_slope     );
		mass_unit          = ModbusDevice::decodeString(du.mid(45, 4)); setData("mass_unit"         , mass_unit         );
		display_brightness = ModbusDevice::decodeUInt16(du.mid(49, 2)); setData("display_brightness", display_brightness);
		uart_baudrate      = ModbusDevice::decodeUInt16(du.mid(51, 2)); setData("uart_baudrate"     , uart_baudrate     );
		rs485_baudrate     = ModbusDevice::decodeUInt16(du.mid(53, 2)); setData("rs485_baudrate"    , rs485_baudrate    );
		rs485_timeout      = ModbusDevice::decodeUInt16(du.mid(55, 2)); setData("rs485_timeout"     , rs485_timeout     );
	}
	else if (rdu == routine_request_du) {
		if (fn != 0x03)
			return ERROR_CODE::HARDWR_ERROR;

		if (du.size() != 9) {
			return ERROR_CODE::UNEXPD_ENTRY;
		}

		mass = ModbusDevice::decodeFloat (du.mid(3 , 4)); setData("mass", mass);

		//dbg(QString("GOT DATA UNIT [%1] AS AN RESPONSE FOR ROUTINE REQUEST [%2]").arg(ModbusDevice::toHumanReadable(du)).arg(ModbusDevice::toHumanReadable(rdu)));
		return ERROR_CODE::NO_ERROR;
	}
	else if (rdu == eeprom_request_du) {
		//dbg(QString("GOT DATA UNIT [%1] AS AN RESPONSE FOR EEPROM REQUEST [%2]").arg(ModbusDevice::toHumanReadable(du)).arg(ModbusDevice::toHumanReadable(rdu)));
		return ERROR_CODE::NO_ERROR;
	}
	else {
		//dbg(QString("GOT DATA UNIT [%1] AS AN RESPONSE FOR UNKNOWN REQUEST").arg(ModbusDevice::toHumanReadable(du)));
		return ERROR_CODE::NO_ERROR;
	}

	return ERROR_CODE::UNEXPD_ENTRY;
}

int SBUnitDevice::setDeviceMode(quint16 m)
{
	device_mode_request_du.clear();
	device_mode_request_du.append(ModbusDevice::encodeUInt16(0x1C)); //Starting register address
	device_mode_request_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	device_mode_request_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	device_mode_request_du.append(ModbusDevice::encodeUInt16(m));
	device_mode_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, device_mode_request_du);
	device_mode = m;
	return request(device_mode_request_du);
}

int SBUnitDevice::saveEEPROM()
{
	eeprom_request_du.clear();
	eeprom_request_du.append(ModbusDevice::encodeUInt8(0)); //X = 0
	eeprom_request_du.append(ModbusDevice::encodeUInt8(0)); //N = 0
	eeprom_request_du = ModbusDevice::dataUnit(modbus_id, 0x41, eeprom_request_du);
	return request(eeprom_request_du);
}

int SBUnitDevice::setTare(float t)
{
	tare = t;
	tare_request_du.clear();
	tare_request_du.append(ModbusDevice::encodeUInt16(0x1F)); //Starting register address
	tare_request_du.append(ModbusDevice::encodeUInt16(2)); //Quantity of registers
	tare_request_du.append(ModbusDevice::encodeUInt8(4)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	tare_request_du.append(ModbusDevice::encodeFloat(t));

	tare_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, tare_request_du);
	return request(tare_request_du);
}

int SBUnitDevice::setTareCurrent()
{
	return setTare(mass);
}

int SBUnitDevice::setTareZero()
{
	return setTare(0);
}

int SBUnitDevice::setCalibration(float a1, float b1, float a2, float b2, float a3, float b3, float a4, float b4) // w = a * x + b
{
	calibration_request_du.clear();
	calibration_request_du.append(ModbusDevice::encodeUInt16(0x21)); //Starting register address
	calibration_request_du.append(ModbusDevice::encodeUInt16(16)); //Quantity of registers
	calibration_request_du.append(ModbusDevice::encodeUInt8(32)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	calibration_request_du.append(ModbusDevice::encodeFloat(a1));
	calibration_request_du.append(ModbusDevice::encodeFloat(b1));
	calibration_request_du.append(ModbusDevice::encodeFloat(a2));
	calibration_request_du.append(ModbusDevice::encodeFloat(b2));
	calibration_request_du.append(ModbusDevice::encodeFloat(a3));
	calibration_request_du.append(ModbusDevice::encodeFloat(b3));
	calibration_request_du.append(ModbusDevice::encodeFloat(a4));
	calibration_request_du.append(ModbusDevice::encodeFloat(b4));

	calibration_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, calibration_request_du);
	return request(calibration_request_du);
}

int SBUnitDevice::setUpdateInterval(quint16 a)
{
	update_interval_request_du.clear();
	update_interval_request_du.append(ModbusDevice::encodeUInt16(0x1D)); //Starting register address
	update_interval_request_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	update_interval_request_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	update_interval_request_du.append(ModbusDevice::encodeUInt16(a));
	update_interval_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, update_interval_request_du);
	return request(update_interval_request_du);
}

int SBUnitDevice::setDisplayBrightness(quint16 a)
{
	display_brightness_request_du.clear();
	display_brightness_request_du.append(ModbusDevice::encodeUInt16(0x33)); //Starting register address
	display_brightness_request_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	display_brightness_request_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	display_brightness_request_du.append(ModbusDevice::encodeUInt16(a));
	display_brightness_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, display_brightness_request_du);
	return request(display_brightness_request_du);
}


float SBUnitDevice::getMass()
{
	return mass;
}

float SBUnitDevice::getTare()
{
	return tare;
}