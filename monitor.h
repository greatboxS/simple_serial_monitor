#ifndef monitor_h
#define monitor_h

#include <stdint.h>
#include <stddef.h>

#define DEFAULT_MONITOR_PORT_NAME "/dev/ttyUSB0"
#define MONITOR_BUFF_MIN_SIZE 1024
#define DEFAULT_MONITOR_BUFFER_SIZE     (1024*5)
#define BAUD_COUNT      18
#define DEFAULT_BAUD    115200

int monitor_init(const char* d_file, int baud);
int monitor_set_buff_size(size_t size);
int monitor_write(uint8_t* buffer, size_t len);
int monitor_read_string(int fd, uint8_t* buf, int timeout);

#endif