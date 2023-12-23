# UNS
Устройство: Raspberry Pi 3 Model B Rev. a02082
Установленный образ: 2023-05-03-raspios-bullseye-armhf-lite

1. Записываем [образ raspbian](https://www.raspberrypi.com/software/operating-systems/) на micro SD карту с помощью [Raspberry Pi Imager](https://www.raspberrypi.com/software/)<br>Я пробовал Debian version: 12 (bookworm) -- по простому не завелась.
3. Нажать на шестерёнку и установить
	* Hostname: `uns`
	* Enable SSH: `true`
	* Username: `ivan`
	* Password: `6177`
	* WLAN SSID: `Engineer`
	* WLAN PASS: `149162536`
4. Вставляем SD карту и подаём питание. Должно подключиться к настроеной сети (удобно раздать со смартфона)
5. Подключиться по SSH
6. Добавить в /etc/dhcpcd.conf:
```
interface eth0
static ip_address=192.168.3.22/24
static routers=192.168.3.1
static domain_name_servers=192.168.1.1 8.8.8.8
```
6. Вставить Ethernet кабель и презагрузиться `sudo reboot`
7. Запускаем `sudo raspi-config`, переходим `Interface Options` и отключаем всё кроме SSH
8. Установить git `sudo apt install git`
9. На текущий момент репозиторий запривачен, поэтому: 
	* Создать пару ключей `ssh-keygen`
	* Добавить публичный ключ `.ssh/id_rsa.pub` в [список разрешенных](https://github.com/settings/keys)
10. Клонировать репозиторий `git clone git@github.com:illepidus/uns.git`
11. Переместиться в склонированный репозиторий `cd ~/uns`
12. Выполняем скрипт первоначальной настройки `sudo init.sh`
13. Если всё в порядке перегружаем на всякий случай `sudo reboot` и работаем
14. Если наблюдаются проблемы с RS485 устройствами, то закрепляем за RS-485<->USB адаптерами их номера
	* Выполняем команды и запоминаем их ID_PATH
	```
	udevadm info /dev/ttyUSB0 | grep ID_PATH=
	udevadm info /dev/ttyUSB1 | grep ID_PATH=
	```
	* `sudo nano /etc/udev/rules.d/99-uns.rules`
	```
	SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-3f980000.usb-usb-0:1.3:1.0", SYMLINK+="ttyRS485-0"
	SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-3f980000.usb-usb-0:1.5:1.0", SYMLINK+="ttyRS485-1"
	```
 15. Перезагружаемся и работаем
