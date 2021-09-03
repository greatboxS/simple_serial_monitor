#ifndef monitor_h
#define monitor_h

#if __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DEFAULT_MONITOR_PORT_NAME "/dev/ttyUSB0"
#define MONITOR_BUFF_MIN_SIZE 1024
#define DEFAULT_MONITOR_BUFFER_SIZE     (1024*5)
#define BAUD_COUNT      18
#define DEFAULT_BAUD    115200

// initilize the monitor
int monitor_init(const char* d_file, int baud);
// start monitor;
int monitor_start();
// stop monitor
void monitor_close();
// enable and disable read and write functions
void monitor_enable_read(bool r);
void monitor_enable_write(bool w);
// set monitor read buffer size
int monitor_set_buff_size(size_t size);
// write a buffer to serial port port
int monitor_write(uint8_t* buffer, size_t len);
// read line from serial port, timeout as milisecond
int monitor_read_string(uint8_t* buf, int timeout); 

#if __cplusplus
}
#endif

#endif