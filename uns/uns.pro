TEMPLATE = app
INCLUDEPATH += src

DESTDIR = bin
OBJECTS_DIR = bin/.obj
MOC_DIR = bin/.moc
RCC_DIR = bin/.rcc
UI_DIR = bin/.ui

SOURCES += src/main.cpp
SOURCES += src/raspberry_gpio.cpp
SOURCES += src/abstract_device.cpp
SOURCES += src/abstract_button_device.cpp
SOURCES += src/abstract_input_device.cpp
SOURCES += src/abstract_modbus_device.cpp
SOURCES += src/abstract_power_device.cpp
SOURCES += src/modbus_device.cpp
SOURCES += src/pi_button_device.cpp
SOURCES += src/pi_power_device.cpp
SOURCES += src/pi_matrix_keyboard_device.cpp
SOURCES += src/sb_unit_device.cpp
SOURCES += src/uns_unit_device.cpp
SOURCES += src/uns_filling_point_device.cpp
SOURCES += src/uns_station_device.cpp
SOURCES += src/event_logger.cpp

HEADERS += src/global.h
HEADERS += src/raspberry_gpio.h
HEADERS += src/abstract_device.h
HEADERS += src/abstract_button_device.h
HEADERS += src/abstract_input_device.h
HEADERS += src/abstract_modbus_device.h
HEADERS += src/abstract_power_device.h
HEADERS += src/modbus_device.h
HEADERS += src/pi_button_device.h
HEADERS += src/pi_power_device.h
HEADERS += src/pi_matrix_keyboard_device.h
HEADERS += src/sb_unit_device.h
HEADERS += src/uns_unit_device.h
HEADERS += src/uns_filling_point_device.h
HEADERS += src/uns_station_device.h
HEADERS += src/event_logger.h

TARGET = uns
CONFIG += release console
QT += xml network serialport
QT -= gui