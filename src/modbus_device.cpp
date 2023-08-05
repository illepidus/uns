#include "modbus_device.h"

ModbusDevice::ModbusDevice(QString n) : AbstractDevice(n)
{
	port = new QSerialPort();

	response_timer = new QTimer(this);
	response_timer->setSingleShot(true);

	frame_timer = new QTimer(this);
	frame_timer->setSingleShot(true);

	statistics_timer = new QTimer(this);
	statistics_timer->setSingleShot(false);

	QObject::connect(response_timer, SIGNAL(timeout()), this, SLOT(unlockWrite()));
	QObject::connect(frame_timer, SIGNAL(timeout()), this, SLOT(processDataUnit()));
	QObject::connect(statistics_timer, SIGNAL(timeout()), this, SLOT(processStatistics()));
	QObject::connect(port, SIGNAL(readyRead()), this, SLOT(readData()));

	valid_data_units = 0;
	invalid_data_units = 0;
	requested_data_units = 0;
}

int ModbusDevice::start()
{
	device_file = "/dev/ttyUSBX";
	baud_rate = 19200;
	frame_timeout = 25;
	response_timeout = 150;

	statistics_interval = 3600000;
	write_lock = false;
	routine_flag = true;
	max_unresponsed_data_units = 10;

	if (property("device_file").isValid())
		device_file = property("device_file").toString(); 
	if (property("baud_rate").isValid())
		baud_rate = property("baud_rate").toInt();
	if (property("frame_timeout").isValid())
		frame_timeout = property("frame_timeout").toInt();
	if (property("response_timeout").isValid())
		response_timeout = property("response_timeout").toInt();
	if (property("statistics_interval").isValid())
		statistics_interval = property("statistics_interval").toInt();
	if (property("max_unresponsed_data_units").isValid())
		max_unresponsed_data_units = property("max_unresponsed_data_units").toInt();

	port->setPortName(device_file);
	port->setBaudRate(baud_rate);
	port->setReadBufferSize(MODBUS_READ_BUFFER_SIZE);

	if (port->open(QIODevice::ReadWrite)) {
		setState(DEVICE_STATE::ON);
	}
	else {
		dbg("Failed to open <" + device_file + ">");
		setState(DEVICE_STATE::ERROR);
		return ERROR_CODE::HARDWR_ERROR;
	}

	if (!statistics_timer->isActive()) {
		statistics_timer->setInterval(statistics_interval);
		statistics_timer->start();
	}

	unresponsed_data_units = 0;
	sendBuf();

	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::stop()
{
	response_timer->stop();
	frame_timer->stop();

	if (!port->isOpen()) {
		dbg("Failed to close <" + device_file + ">");
		setState(DEVICE_STATE::OFF);
		return ERROR_CODE::HARDWR_ERROR;
	}
	port->close();
	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::restart()
{
	dbg("RESTARTING...");
	int e;
	e = stop();
	if (e != ERROR_CODE::NO_ERROR) {
		dbg(QString("Failed to stop: %1").arg(e));
	}
	e = start();
	if (e != ERROR_CODE::NO_ERROR) {
		dbg(QString("Failed to start: %1").arg(e));
		return e;
	}
	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::detonate() {
	dbg("DETONATION");

	return ERROR_CODE::NO_ERROR;
}

QString ModbusDevice::currentStatus() {
	QString readable_send_buf;
	QString readable_rout_buf;
	for (int i = 0; i < send_buf.size();    ++i) readable_send_buf += ("[" + toHumanReadable(send_buf.at(i)   ) + "]");
	for (int i = 0; i < routine_buf.size(); ++i) readable_rout_buf += ("[" + toHumanReadable(routine_buf.at(i)) + "]");
	return (QString("[%1].\nRemaining response time = %2[ms].\nRemaining frame time = %3[ms].\nread_buf = %4.\nsend_buf = %5.\nroutine_buf = %6.\nwrite_lock = %7, routine_flag = %8")
		.arg(getName())
		.arg(response_timer->remainingTime())
		.arg(frame_timer->remainingTime())
		.arg(toHumanReadable(read_buf))
		.arg(readable_send_buf)
		.arg(readable_rout_buf)
		.arg(write_lock)
		.arg(routine_flag));
}

int ModbusDevice::readData() 
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	read_buf = read_buf + port->readAll();
	frame_timer->start(frame_timeout);
	response_timer->stop();

	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::sendBuf()
{
	if (unresponsed_data_units >= max_unresponsed_data_units)
		return restart();
	if ((send_buf.size() < 1) && (routine_buf.size() < 1))
		return ERROR_CODE::BROKEN_ENTRY;
	if (write_lock)
		return ERROR_CODE::ACTION_PERMT;
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	QByteArray du;
	if (send_buf.size() > 0) {
		du = send_buf.at(0);
		routine_flag = false;
	}
	else if (routine_buf.size() > 0) {
		du = routine_buf.at(0);
		routine_flag = true;
		routine_buf.append(routine_buf.takeFirst());
	}

	lockWrite();
	int n = port->write(du);
	if (n < du.size()) dbg(QString("ERROR writing data unit [%1]. %2 bytes were written.").arg(ModbusDevice::toHumanReadable(du)).arg(n));

	//dbg(QString("W: %1").arg(ModbusDevice::toHumanReadable(du)));
	requested_data_units++;
	unresponsed_data_units++;
	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::lockWrite()
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	write_lock = true;
	response_timer->start(response_timeout);
	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::unlockWrite()
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	write_lock = false;
	return sendBuf();
}

int ModbusDevice::processDataUnit()
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	//if (read_buf.size() > 0) dbg(QString("R: %1").arg(ModbusDevice::toHumanReadable(read_buf)));
	frame_timer->stop();
	if (checkDataUnit(read_buf)) {
		valid_data_units++;
		unresponsed_data_units = 0;
		emit dataUnitRecieved(read_buf);
		if (routine_flag) {
			emit dataUnitRecieved(read_buf, routine_buf.at(routine_buf.count() - 1));
		}
		else {
			emit dataUnitRecieved(read_buf, send_buf.at(0));
			send_buf.removeFirst();
		}
	}
	else invalid_data_units++;

	unlockWrite();
	read_buf.clear();

	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::processStatistics()
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	dbg(QString("Recieved %1/%2(%3%) of requested data units per last %4[sec]. %5/%1(%6%) of it was valid.")
		.arg(valid_data_units + invalid_data_units, 2)
		.arg(requested_data_units, 2)
		.arg(100 * (valid_data_units + invalid_data_units) / requested_data_units, 3)
		.arg(statistics_interval/1000)
		.arg(valid_data_units, 2)
		.arg(100 * valid_data_units / (valid_data_units + invalid_data_units), 3)
	);

	valid_data_units = 0;
	invalid_data_units = 0;
	requested_data_units = 0;
	return ERROR_CODE::NO_ERROR;
}

int ModbusDevice::sendDataUnit(QByteArray du)
{
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	if (send_buf.size() >= MODBUS_BUFFER_OVERFLOW_SIZE)  {
		dbg("Send buffer overflow. Turning device to error state.");
		setState(DEVICE_STATE::ERROR);
	}

	send_buf += du;
	return sendBuf();
}

int ModbusDevice::addRoutine(QByteArray du) {
	if (state != DEVICE_STATE::ON)
		return ERROR_CODE::BAD_DV_STATE;

	routine_buf += du;
	return sendBuf();
}

//++++++++++++++++++STATIC FUNCTIONS+++++++++++++++++

quint16 ModbusDevice::crc16(QByteArray du) {
	quint8 nTemp;
	quint16 wCRCWord = 0xFFFF;
	foreach(nTemp, du) {
		nTemp = nTemp ^ wCRCWord;
		wCRCWord >>= 8;
		wCRCWord ^= wCRCTable[nTemp];
	}

	return wCRCWord;
}

QString ModbusDevice::toString(QByteArray du) 
{
	QString echo;
	QString byte;
	quint8 c;
	foreach(c, du) {
		if (c > 0x0F)
			echo += byte.setNum(c, 16).toUpper() + " ";
		else
			echo += "0" + byte.setNum(c, 16).toUpper() + " ";
	}
	echo.chop(1);
	return echo;
}

bool ModbusDevice::checkDataUnit(QByteArray du) 
{
	if (crc16(du) == 0)
		return true;
	return false;
}

quint8 ModbusDevice::getID(QByteArray du)
{
	return du.at(0);
}

uint8_t ModbusDevice::getFnCode(QByteArray du)
{
	return du.at(1);
}

QByteArray ModbusDevice::getData(QByteArray du)
{
	return du.mid(2, du.size()-4);
}

qint32 ModbusDevice::decodeInt32(QByteArray data)
{
	if (data.size() != sizeof(qint32))
		return 0;

	data = swapByteOrder(data);

	qint32 r;
	quint8 i[sizeof(r)];
	for (uint j=0; j<sizeof(r); j++) {
		i[j] = data.at(j);
	}
	memcpy (&r, &i, sizeof(i));
	return r;
}


quint32 ModbusDevice::decodeUInt32(QByteArray data)
{
	if (data.size() != sizeof(quint32))
		return 0;

	data = swapByteOrder(data);

	quint32 r;
	quint8 i[sizeof(r)];
	for (uint j=0; j<sizeof(r); j++) {
		i[j] = data.at(j);
	}
	memcpy (&r, &i, sizeof(i));
	return r;
}


qint16 ModbusDevice::decodeInt16(QByteArray data)
{
	if (data.size() != sizeof(qint16))
		return 0;

	data = swapByteOrder(data);

	qint16 r = 0;
	quint8 i[sizeof(r)];

	for (uint j=0; j < sizeof(r); j++) {
		i[j] = data.at(j);
	}
	memcpy (&r, &i, sizeof(i));	
	return r;
}

quint16 ModbusDevice::decodeUInt16(QByteArray data)
{
	if (data.size() != sizeof(quint16))
		return 0;

	data = swapByteOrder(data);

	quint16 r = 0;
	quint8 i[sizeof(r)];

	for (uint j=0; j < sizeof(r); j++) {
		i[j] = data.at(j);
	}
	memcpy (&r, &i, sizeof(i));	
	return r;
}

float ModbusDevice::decodeFloat(QByteArray data) 
{
	if (data.size() != sizeof(float))
		return 0;

	data = swapByteOrder(data);

	float r = 0;
	quint8 i[sizeof(r)];

	for (uint j=0; j<sizeof(r); j++)
		i[j] = data.at(j);

	memcpy (&r, &i, sizeof(r));
	if (r != r) //is NaN??
		return 0;

	return r;
}


QString ModbusDevice::decodeString(QByteArray data) 
{
	data = swapByteOrder(data);

	QString r = QString::fromLatin1(data);
	return r;
}

QByteArray ModbusDevice::encodeInt32(qint32 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	res = swapByteOrder(res);
	return res;
}


QByteArray ModbusDevice::encodeUInt32(quint32 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	res = swapByteOrder(res);
	return res;
}

QByteArray ModbusDevice::encodeInt16(qint16 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	res = swapByteOrder(res);
	return res;
}

QByteArray ModbusDevice::encodeUInt16(quint16 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	res = swapByteOrder(res);
	return res;
}

QByteArray ModbusDevice::encodeInt8(qint8 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	return res;
}

QByteArray ModbusDevice::encodeUInt8(quint8 n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	return res;
}

QByteArray ModbusDevice::encodeFloat(float n)
{
	QByteArray res;
	quint8 i[sizeof(n)];
	memcpy (&i, &n, sizeof(n));
	for (uint j=0; j<sizeof(n); j++) {
		res += i[j];
	}
	res = swapByteOrder(res);
	return res;
}

QByteArray ModbusDevice::encodeString(QString s)
{
	QByteArray res = s.toLatin1();
	res = swapByteOrder(res);
	return res;
}


QByteArray ModbusDevice::dataUnit(quint8 id, quint8 fn, QByteArray data)
{
	QByteArray du;
	du += id;
	du += fn;
	du += data;

	quint32 crc = crc16(du);
	du+=crc & 0xFF;
	du+=crc >> 8 & 0xFF;

	return du;
}

QByteArray ModbusDevice::dataUnit(quint8 id, quint8 fn)
{
	QByteArray du;
	du += id;
	du += fn;

	quint32 crc = crc16(du);
	du+=crc & 0xFF;
	du+=crc >> 8 & 0xFF;

	return du;
}

QByteArray ModbusDevice::reverseBytes(QByteArray data)
{
	QByteArray buf = data;
	for (int i=0; i<data.size(); i++) {
		quint8 low = data[i] >> 4 & 0xF;
		quint8 hig = data[i] & 0xF;
		buf[i] = (hig << 4) + low;
	}
	return buf;
}

QByteArray ModbusDevice::reverseByteOrder(QByteArray data)
{
	QByteArray buf = data;
	for (int i=0; i<data.size(); i++) {
		buf[i] = data[data.size()-i-1];
	}
	return buf;
}

QByteArray ModbusDevice::swapByteOrder(QByteArray data)
{
	if (data.size() % 2 != 0) {
		return data;
	}

	QByteArray buf = data;
	for (int i=0; i < data.size(); i+=2) {
		buf[i] = data[i+1];
		buf[i+1] = data[i];
	}
	return buf;
}

QString ModbusDevice::toHumanReadable(QByteArray data)
{
	QString r;
	quint8 c;
	foreach(c, data) {
		if (c < 0x10)
			r += QString("0");
		r += QString::number(c, 16).toUpper();
		r += QString(" ");
	}
	r.chop(1);
	return r;
}
