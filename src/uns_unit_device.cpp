#include "uns_unit_device.h"

UNSUnitDevice::UNSUnitDevice(QString n, ModbusDevice *b) : AbstractModbusDevice(n, b)
{
	QTimer::singleShot(3000, this, SLOT(detonate()));
}


int UNSUnitDevice::detonate()
{

	return ERROR_CODE::NO_ERROR;
}


int UNSUnitDevice::hardwareInit()
{
	state_stable = false;
	state_rising = false;
	state_lowering = false;
	device_state = 0x00;
	value = 0;
	mass = 0.0;
	tare = 0.0;

	initial_request_du.append((char) 0x00);
	initial_request_du.append((char) 0x1C);
	initial_request_du.append((char) 0x00);
	initial_request_du.append((char) 0x15);  //0x3B [59] is MAXIMUM. NEVER OVERLIMIT IT. OTHERWISE SHIT WOULD HAPPEN;
	initial_request_du = ModbusDevice::dataUnit(modbus_id, 0x03, initial_request_du);
	request(initial_request_du);

	routine_request_du.append((char) 0x00);
	routine_request_du.append((char) 0x82);
	routine_request_du.append((char) 0x00);
	routine_request_du.append((char) 0x05);
	routine_request_du = ModbusDevice::dataUnit(modbus_id, 0x03, routine_request_du);
	addRoutine(routine_request_du);


	return ERROR_CODE::NO_ERROR;
}

int UNSUnitDevice::hardwareProcessDataUnit(QByteArray du, QByteArray rdu)
{
	int fn = ModbusDevice::getFnCode(du);

	if (rdu == initial_request_du) {
		if (fn != 0x03)
			return ERROR_CODE::HARDWR_ERROR;

		if (du.size() != 47) {
			dbg("REREQUESTING INITIALIZATION DATA");
			request(initial_request_du);
			return ERROR_CODE::UNEXPD_ENTRY;
		}

		device_mode        = ModbusDevice::decodeUInt16(du.mid(3, 2 ));  setData("device_mode"       , device_mode       );
		update_interval    = ModbusDevice::decodeUInt16(du.mid(5, 2 ));  setData("update_interval"   , update_interval   );
		hits_per_weight    = ModbusDevice::decodeUInt16(du.mid(7, 2 ));  setData("hits_per_weight"   , hits_per_weight   );
		scales1_intercept  = ModbusDevice::decodeFloat (du.mid(9, 4 ));  setData("scales1_intercept" , scales1_intercept );
		scales1_slope      = ModbusDevice::decodeFloat (du.mid(13, 4 )); setData("scales1_slope"     , scales1_slope     );
		scales2_intercept  = ModbusDevice::decodeFloat (du.mid(17, 4 )); setData("scales2_intercept" , scales2_intercept );
		scales2_slope      = ModbusDevice::decodeFloat (du.mid(21, 4 )); setData("scales2_slope"     , scales2_slope     );
		mass_unit          = ModbusDevice::decodeString(du.mid(25, 4 )); setData("mass_unit"         , mass_unit         );
		display_brightness = ModbusDevice::decodeUInt16(du.mid(29, 2 )); setData("display_brightness", display_brightness);
		uart_baudrate      = ModbusDevice::decodeUInt16(du.mid(31, 2 )); setData("uart_baudrate"     , uart_baudrate     );
		rs485_baudrate     = ModbusDevice::decodeUInt16(du.mid(33, 2 )); setData("rs485_baudrate"    , rs485_baudrate    );
		rs485_timeout      = ModbusDevice::decodeUInt16(du.mid(35, 2 )); setData("rs485_timeout"     , rs485_timeout     );
//		modbus_id          = ModbusDevice::decodeUInt16(du.mid(37, 2 )); setData("modbus_id"         , modbus_id         );
		mass_stable_count  = ModbusDevice::decodeUInt16(du.mid(39, 2 )); setData("mass_stable_count" , mass_stable_count );
		mass_stable_delta  = ModbusDevice::decodeFloat (du.mid(41, 4 )); setData("mass_stable_delta" , mass_stable_delta );

		setTareZero(); //Иначе может расходиться показания на приборе и в памяти программы
	}
	else if (rdu == routine_request_du) {
		if (fn != 0x03)
			return ERROR_CODE::HARDWR_ERROR;

		if (du.size() != 15)
			return ERROR_CODE::UNEXPD_ENTRY;

		device_state = ModbusDevice::decodeUInt16(du.mid(3 , 2));
		value        = ModbusDevice::decodeInt32 (du.mid(5 , 4));
		mass         = ModbusDevice::decodeFloat (du.mid(9 , 4));

		setData("value", value);
		setData("mass",  mass);
		setData("tare",  tare);
		setData("device_state", device_state);
		setData("display_mode",  getDisplayMode());
		setData("active_scales",  getActiveScales());

		state_stable   = (device_state & 0b001) ? true : false;
		state_rising   = (device_state & 0b010) ? true : false;
		state_lowering = (device_state & 0b100) ? true : false;

		return ERROR_CODE::NO_ERROR;
	}


	return ERROR_CODE::NO_ERROR;
}


int UNSUnitDevice::saveEEPROM()
{
	eeprom_request_du.clear();
	eeprom_request_du.append(ModbusDevice::encodeUInt8(0)); //X = 0
	eeprom_request_du.append(ModbusDevice::encodeUInt8(0)); //N = 0
	eeprom_request_du = ModbusDevice::dataUnit(modbus_id, 0x41, eeprom_request_du);
	return request(eeprom_request_du);
}


int UNSUnitDevice::softwareReset()
{
	reset_request_du.clear();
	reset_request_du.append(ModbusDevice::encodeUInt8(1)); //X = 1
	reset_request_du.append(ModbusDevice::encodeUInt8(0)); //N = 0
	reset_request_du = ModbusDevice::dataUnit(modbus_id, 0x41, reset_request_du);
	return request(reset_request_du);
}

int UNSUnitDevice::displayWeight()
{
	display_request_du.clear();
	display_request_du.append(ModbusDevice::encodeUInt8(4)); //X = 4
	display_request_du.append(ModbusDevice::encodeUInt8(0)); //N = 0
	display_request_du = ModbusDevice::dataUnit(modbus_id, 0x41, display_request_du);

	display_text = "";
	setData("display_text", "");
	return request(display_request_du);
}

int UNSUnitDevice::display(QString s, bool c)
{
	display_request_du.clear();
	display_request_du.append(ModbusDevice::encodeUInt8(5)); //X
	display_request_du.append(ModbusDevice::encodeUInt8(5)); //N
	display_request_du.append(ModbusDevice::encodeString(s.rightJustified(4, ' ').right(4))); //SSSS
	display_request_du.append(ModbusDevice::encodeUInt8(c ? 1 : 0)); //K
	display_request_du = ModbusDevice::dataUnit(modbus_id, 0x41, display_request_du);

	display_text = s;
	setData("display_text", s);
	return request(display_request_du);
}

int UNSUnitDevice::setTare(float t)
{
	int s = getActiveScales();
	if ((s < 1) || (s > 2))
		return ERROR_CODE::BAD_DV_STATE;

	tare_request_du.clear();
	tare_request_du.append((s == 1) ? ModbusDevice::encodeUInt16(0x92) : ModbusDevice::encodeUInt16(0x98)); //Starting register address
	tare_request_du.append(ModbusDevice::encodeUInt16(2)); //Quantity of registers
	tare_request_du.append(ModbusDevice::encodeUInt8(4)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	tare_request_du.append(ModbusDevice::encodeFloat(t));
	tare_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, tare_request_du);

	tare = t;
	setData("tare",  tare);
	return request(tare_request_du);
}

int UNSUnitDevice::setTareCurrent()
{
	return setTare(mass);
}

int UNSUnitDevice::setTareZero()
{
	return setTare(0);
}


int UNSUnitDevice::setCalibration(float a, float b) // w = a * x + b
{
	int s = getActiveScales();
	if ((s < 1) || (s > 2))
		return ERROR_CODE::BAD_DV_STATE;

	calibration_request_du.clear();
	calibration_request_du.append((s == 1) ? ModbusDevice::encodeUInt16(0x1F) : ModbusDevice::encodeUInt16(0x23)); //Starting register address
	calibration_request_du.append(ModbusDevice::encodeUInt16(4)); //Quantity of registers
	calibration_request_du.append(ModbusDevice::encodeUInt8(8)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	calibration_request_du.append(ModbusDevice::encodeFloat(b));
	calibration_request_du.append(ModbusDevice::encodeFloat(a));
	calibration_request_du = ModbusDevice::dataUnit(modbus_id, 0x10, calibration_request_du);
	if (s == 1) {
		scales1_slope = a;
		scales1_intercept = b;
		setData("scales1_slope", a);
		setData("scales1_intercept", b);
	}
	else {
		scales2_slope = a;
		scales2_intercept = b;
		setData("scales2_slope", a);
		setData("scales2_intercept", b);
	}
	return request(calibration_request_du);
}


int UNSUnitDevice::setMassStableDelta(float a)
{
	mass_stable_delta_du.clear();
	mass_stable_delta_du.append(ModbusDevice::encodeUInt16(0x2F)); //Starting register address
	mass_stable_delta_du.append(ModbusDevice::encodeUInt16(2)); //Quantity of registers
	mass_stable_delta_du.append(ModbusDevice::encodeUInt8(4)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	mass_stable_delta_du.append(ModbusDevice::encodeFloat(a));
	mass_stable_delta_du = ModbusDevice::dataUnit(modbus_id, 0x10, mass_stable_delta_du);
	mass_stable_delta = a;
	setData("mass_stable_delta", a);
	return request(mass_stable_delta_du);
}

int UNSUnitDevice::setMassStableCount(quint16 a)
{
	mass_stable_count_du.clear();
	mass_stable_count_du.append(ModbusDevice::encodeUInt16(0x2E)); //Starting register address
	mass_stable_count_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	mass_stable_count_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	mass_stable_count_du.append(ModbusDevice::encodeUInt16(a));
	mass_stable_count_du = ModbusDevice::dataUnit(modbus_id, 0x10, mass_stable_count_du);

	mass_stable_count = a;
	setData("mass_stable_count", a);
	return request(mass_stable_count_du);
}


int UNSUnitDevice::setUpdateInterval(quint16 a)
{
	update_interval_du.clear();
	update_interval_du.append(ModbusDevice::encodeUInt16(0x1D)); //Starting register address
	update_interval_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	update_interval_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	update_interval_du.append(ModbusDevice::encodeUInt16(a));
	update_interval_du = ModbusDevice::dataUnit(modbus_id, 0x10, update_interval_du);

	update_interval = a;
	setData("update_interval", a);
	return request(update_interval_du);
}

int UNSUnitDevice::setDisplayBrightness(quint16 a)
{
	display_brightness_du.clear();
	display_brightness_du.append(ModbusDevice::encodeUInt16(0x29)); //Starting register address
	display_brightness_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	display_brightness_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	display_brightness_du.append(ModbusDevice::encodeUInt16(a));
	display_brightness_du = ModbusDevice::dataUnit(modbus_id, 0x10, display_brightness_du);

	display_brightness = a;
	setData("display_brightness", a);
	return request(display_brightness_du);
}

int UNSUnitDevice::setActiveScales(int a) 
{
	if (a == 1) {
		device_mode = device_mode & 0b1111111111111100;
		device_mode = device_mode | 0b0000000000000001;
	}
	else if (a == 2) {
		device_mode = device_mode & 0b1111111111111100;
		device_mode = device_mode | 0b0000000000000010;
	}
	else return ERROR_CODE::UNEXPD_ENTRY;

	active_scales_du.clear();
	active_scales_du.append(ModbusDevice::encodeUInt16(0x1C)); //Starting register address
	active_scales_du.append(ModbusDevice::encodeUInt16(1)); //Quantity of registers
	active_scales_du.append(ModbusDevice::encodeUInt8(2)); //Quantity of bytes = 2 * registers [WTF? REALLY?]
	active_scales_du.append(ModbusDevice::encodeUInt16(device_mode));
	active_scales_du = ModbusDevice::dataUnit(modbus_id, 0x10, active_scales_du);

	setData("device_mode", device_mode);
	return request(active_scales_du);
}


int UNSUnitDevice::getActiveScales() 
{
	bool h1_f = ((device_mode & 0x01) != 0);
	bool h2_f = ((device_mode & 0x02) != 0);

	if (h1_f & (!h2_f))
		return 1;
	if (h2_f & (!h1_f))
		return 2;
	return 0;
}

int UNSUnitDevice::getDisplayMode() 
{
	return ((device_mode & 0x04) != 0) ? 1 : 0;
}