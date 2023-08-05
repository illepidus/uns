#pragma once
#include "global.h"
#include "abstract_device.h"

class AbstractButtonDevice : public AbstractDevice
{
	Q_OBJECT

	public:
		AbstractButtonDevice(QString);
	signals:
		void clicked();
		void longClicked();
		void doubleClicked();
		void pressed();
		void released();
	public slots:
		int start();
		int stop();
		int click();
		int longClick();
		int doubleClick();
	protected slots:
		int up();
		int down();
	protected slots:
		virtual int hardwareInit();
		virtual int hardwareDeinit();
	protected:
		QTime  *click_timer;
		QTimer *double_click_timer;
		int click_interval; //Время, в течении которого кнопка должна быть в положении DOWN, чтобы клик засчитался.
		int long_click_interval; //Время в течении которого кнопка должна быть в положении DOWN, чтобы клик засчитался за LONG.
		int double_click_interval; //Время, в течении которого будет ожидаться второй клик после отпускания кнопки.
		bool first_click;
};