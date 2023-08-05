#pragma once
#include "global.h"
#include "raspberry_gpio.h"
#include "abstract_input_device.h"

const static QChar STANDART_KEYCODE_TABLE [] = {
	'1', '2', '3', 'A', 
	'4', '5', '6', 'B',
	'7', '8', '9', 'C', 
	'*', '0', '#', 'D'
};

class PiMatrixKeyboardDevice : public AbstractInputDevice
{
	Q_OBJECT

	public:
		PiMatrixKeyboardDevice(QString);
	signals:
		void cancel();
		void keyPress(QChar, QString);
		void command(QString);
	private slots:
		int handleInterrupt(QString);
		int hardwareInit();
		int hardwareDeinit();
		int processButtonPress();
		int pressButton(int);
	private:
		bool pull_up;
		QVector<int> in_pins;
		QVector<int> out_pins;
		QVector<QChar> key_codes;
		QChar command_terminator_char;
		QChar command_cancel_char;
		QFileSystemWatcher watcher;
		QTextStream text_stream;
		QTimer bounce_timer;
		QString command_line;
		int bounce_time;
		int current_row;
		int num_rows;
		int num_columns;
		int command_terminator;
		int command_cancel;
		int max_command_size;
};