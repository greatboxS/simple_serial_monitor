# simple_serial_monitor

Simple serial monitor, using read, wirte device method

# Command:
        ./monitor -f </dev/port_name> -b <baud_rate> -a <byte>
        
e.g:    ./monitor -f /dev/ttyUSB0 -b 115200 -a 1024

# additional cmds:
        -m start        -start monitor
        -m stop         -stop monitor
        -r dis          -disable read
        -r ena          -enable read / have to enable after use -r line 

        -r line <num>   -read <num> line from serial port > press "e" / "q" to cancel 
                        -read line function disable monitor func
                        -enable read after use -r line <num> function
        -w dis          -diable write
        -w ena          -enable write

# build:
gcc -o monitor monitor_app.c monitor.h monitor.c -lpthread
