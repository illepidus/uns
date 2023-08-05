//DEPENDS ON WiringPi LIBRARY
#pragma once
#include "global.h"

#define PI_GPIO_INPUT 0
#define PI_GPIO_OUTPUT 1
#define PI_GPIO_EDGE_NONE 0
#define PI_GPIO_EDGE_FALLING 1
#define PI_GPIO_EDGE_RISING 2
#define PI_GPIO_EDGE_BOTH 3
#define PI_GPIO_MODE_DOWN 0
#define PI_GPIO_MODE_UP 1
#define PI_GPIO_MODE_TRI 2
#define PI_GPIO_PIN_MIN 2
#define PI_GPIO_PIN_MAX 27
#define PI_GPIO_PIN_INVALID 0
#define PI_GPIO_PATTERN_VALUE      "/sys/class/gpio/gpio(\\d+)/value"
#define PI_GPIO_PATH_EXPORT        "/sys/class/gpio/export"
#define PI_GPIO_PATH_UNEXPORT      "/sys/class/gpio/unexport"
#define PI_GPIO_TEMPLATE_EDGE      "/sys/class/gpio/gpio%1/edge"
#define PI_GPIO_TEMPLATE_VALUE     "/sys/class/gpio/gpio%1/value"
#define PI_GPIO_TEMPLATE_DIRECTION "/sys/class/gpio/gpio%1/direction"
#define PI_GPIO_TEMPLATE_MODE      "gpio -g mode %1 %2"

class RaspberryGPIO : public QObject {
	Q_OBJECT
	public slots:
		static void detonate();
		static bool checkPinRange(int);
		static int initPin(int);
		static int deinitPin(int);
		static int setPinDirection(int, int);
		static int setPinInput(int);
		static int setPinOutput(int);
		static int setPinInterrupt(int, int);
		static int setPinInterruptRising(int);
		static int setPinInterruptFalling(int);
		static int setPinInterruptAll(int);
		static int setPinInterruptNone(int);
		static int getPinValue(int, int&);
		static int setPinValue(int, int);
		static int setPinOn(int);
		static int setPinOff(int);
		static int setPinMode(int, int);
		static int setPinModeUp(int);
		static int setPinModeDown(int);
		static int setPinModeTri(int);
};