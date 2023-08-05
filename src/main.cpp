#include "global.h"
#include "uns_station_device.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
	qDebug() << "APPLICATION STARTED!";
	QCoreApplication app(argc, argv);
	//if (!QCoreApplication::arguments().contains("--no-daemon")) daemon(0, 1);
	UNSStationDevice station("UNS");
	station.start();
	//QTimer::singleShot(100, &app, SLOT(quit()));
	return app.exec();
}