#pragma once
#include <QtCore>
#include <typeinfo>
#include <stdlib.h>
#include <stdint.h>

#define STRINGIFY(x) #x
#define DATETIME QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss")
#define DATE     QDateTime::currentDateTime().toString("dd.MM.yy")
#define TIME     QDateTime::currentDateTime().toString("hh:mm:ss")

#define SETTINGS_FILE_PATH "/etc/uns_settings"
#define DATA_FILE_PATH "/etc/uns_data"
#define EVENT_LOG_PATH QString("/var/www/log/uns_%1.xml").arg(QDate::currentDate().toString("yyMMdd"))

#define EXIT_CODE_MODBUS_ERROR 1

namespace DEVICE_STATE {
	enum DEVICE_STATE {
		OFF,
		ON,
		UNKNOWN,
		ERROR,
		HANGED,
		WAITING,
		UP,
		DOWN,
		EMPTY,
		MAINTENANCE,
		EQUIPPED,
		READY,
		OVERWEIGHT,
		STOPPED,
		DISABLED
	};
}

namespace ERROR_CODE {
	enum ERROR_CODE {
		NO_ERROR,
		FATAL_ERROR,
		GENERL_ERROR,
		HARDWR_ERROR,
		UNKNWN_ENTRY,
		DUPCAT_ENTRY,
		UNEXPD_ENTRY,
		BROKEN_ENTRY,
		NULL_POINTER,
		NO_SUCH_FILE,
		OUT_OF_RANGE,
		BAD_DV_STATE,
		MISING_VARBL,
		ACTION_PERMT,
		UNSTABLE_VAL
	};
}


