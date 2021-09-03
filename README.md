# simple_serial_monitor

Simple serial monitor, using read, wirte device method

Command:
        ./monitor -f </dev/port_name> -b <baud_rate> -a <byte>
        
e.g:    ./monitor -f /dev/ttyUSB0 -b 115200 -a 1024

build:
gcc -o monitor monitor_app.c monitor.h monitor.c -lpthread
