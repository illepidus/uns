#Закрепляем за RS-485<->USB адаптерами их номера
1. Выполняем команды
```
udevadm info /dev/ttyUSB0 | grep ID_PATH=`
udevadm info /dev/ttyUSB1 | grep ID_PATH=`
```
И запоминаем их ID_PATH
2. `sudo nano /etc/udev/rules.d/50-uns.rules`
```
SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-3f980000.usb-usb-0:1.3:1.0", SYMLINK+="ttyRS485-0"
SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-3f980000.usb-usb-0:1.5:1.0", SYMLINK+="ttyRS485-1"
```