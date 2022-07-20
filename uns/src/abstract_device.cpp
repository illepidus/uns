#include "abstract_device.h"

AbstractDevice::AbstractDevice(QString n)
{
	device_name = n;
	device_settings = new QSettings(SETTINGS_FILE_PATH, QSettings::IniFormat);
	device_data = new QSettings(DATA_FILE_PATH, QSettings::IniFormat);
	previous_state = DEVICE_STATE::UNKNOWN;
	state = DEVICE_STATE::UNKNOWN; //ONLY time we do it NOT via setState()
	loadSettings();
}

int AbstractDevice::start()
{
	setState(DEVICE_STATE::ON);
	return ERROR_CODE::NO_ERROR;
}

int AbstractDevice::stop() {
	setState(DEVICE_STATE::OFF);
	return ERROR_CODE::NO_ERROR;
}

QString AbstractDevice::getName()
{
	return device_name;
}

QString AbstractDevice::getClass()
{
	if (property("class").isValid())
		device_class = property("class").toString();
	else
		device_class = this->metaObject()->className();
	return device_class;
}

QVariantList AbstractDevice::getSlots()
{
	const QMetaObject* metaObject = this->metaObject();
	QVariantList methods;

	for(int i = 0; i < metaObject->methodCount(); ++i) {
		if ((metaObject->method(i).methodType() == QMetaMethod::Slot) && (metaObject->method(i).access() == QMetaMethod::Public))
			methods << QString::fromLatin1(metaObject->method(i).methodSignature());
	}

	return methods;
}

int AbstractDevice::getState()
{
	return state;
}

int AbstractDevice::getPreviousState()
{
	return previous_state;
}

int AbstractDevice::setState(int s)
{
	if (state != s) {
		previous_state = state;
		state = s;
		setData("state", s);
		emit stateChanged(s);
		emit stateChanged();
	}
	return ERROR_CODE::NO_ERROR;
}


int AbstractDevice::setData(QString s, QVariant v)
{
	/*У QT проблемы с float. Давайте поможем ему!*/
	if ((int)v.type() == (int)QMetaType::Float) v = QVariant(v.toDouble());

	device_data->setValue((device_name + "/" + s).toUtf8(), v);
	setProperty(s.toUtf8(), v);
	emit dataSaved();
	return ERROR_CODE::NO_ERROR;
}

int AbstractDevice::setSetting(QString s, QVariant v)
{
	device_settings->setValue((device_name + "/" + s).toUtf8(), v);
	setProperty(s.toUtf8(), v);
	emit settingSaved();
	return ERROR_CODE::NO_ERROR;
}

QVariant AbstractDevice::getData(QString s)
{
	if (property(s.toUtf8()).isValid())
		return property(s.toUtf8());

	QVariant r;
	r = device_data->value((getName() + "/" + s).toUtf8());
	return r;
}

QVariant AbstractDevice::getSetting(QString s)
{
	return property(s.toUtf8());
}

void AbstractDevice::dbg(QString s)
{
	qDebug() << DATETIME << device_name << s;
}

int AbstractDevice::detonate() 
{
	dbg("DETONATE");
	return ERROR_CODE::NO_ERROR;
}

int AbstractDevice::loadSettings()
{
	device_settings->beginGroup(getName());
		foreach (const QString &key, device_settings->allKeys()) {
			setProperty(key.toUtf8(), device_settings->value(key));
		}
	device_settings->endGroup();
	return ERROR_CODE::NO_ERROR;
}

QString AbstractDevice::getStateName(int s)
{
	if (s == DEVICE_STATE::OFF)         return QString("OFF");
	if (s == DEVICE_STATE::ON)          return QString("ON");
	if (s == DEVICE_STATE::UP)          return QString("UP");
	if (s == DEVICE_STATE::DOWN)        return QString("OFF");
	if (s == DEVICE_STATE::ERROR)       return QString("ERROR");
	if (s == DEVICE_STATE::HANGED)      return QString("HANGED");
	if (s == DEVICE_STATE::UNKNOWN)     return QString("UNKNOWN");
	if (s == DEVICE_STATE::WAITING)     return QString("WAITING");
	if (s == DEVICE_STATE::EMPTY)       return QString("EMPTY");
	if (s == DEVICE_STATE::MAINTENANCE) return QString("MAINTENANCE");
	if (s == DEVICE_STATE::EQUIPPED)    return QString("EQUIPPED");
	if (s == DEVICE_STATE::READY)       return QString("READY");
	if (s == DEVICE_STATE::OVERWEIGHT)  return QString("OVERWEIGHT");
	if (s == DEVICE_STATE::STOPPED)     return QString("STOPPED");
	return QString("*********");
}
