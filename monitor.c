#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "monitor.h"
#include <threads.h>
#include <stdbool.h>

const speed_t baud_rates[BAUD_COUNT][2] = {
    {9600, B9600},
    {19200, B19200},
    {38400, B38400},
    {57600, B57600},
    {115200, B115200},
    {230400, B230400},
    {460800, B460800},
    {500000, B500000},
    {576000, B576000},
    {921600, B921600},
    {1000000, B1000000},
    {1152000, B1152000},
    {1500000, B1500000},
    {2000000, B2000000},
    {2500000, B2500000},
    {3000000, B3000000},
    {3500000, B3500000},
    {4000000, B4000000}};

static thrd_t monitor_thread;
static mtx_t mutex;
static int fd = -1;
static char file[256] = {0};
static speed_t monitor_baud = DEFAULT_BAUD;
static uint8_t *monitor_buffer = NULL;
static int monitor_buff_size = DEFAULT_MONITOR_BUFFER_SIZE;
static bool is_open = false;
static bool e_read = false;
static bool e_write = false;
static bool is_init = false;
static bool is_thread_start = false;
static speed_t get_baud_code(speed_t b);
static int monitor_thread_func(void *arg);
static void monitor_config(int fd, speed_t baud);
static int monitor_open();

// Initialize the monitor first to start monitor
int monitor_init(const char *d_file, int baud)
{
    monitor_baud = get_baud_code(baud);
    memccpy(file, d_file, 0, strlen(d_file));

    if (monitor_open() < 0)
        return -1;

    // config serial port
    monitor_config(fd, monitor_baud);

    // create mutex
    if (mtx_init(&mutex, mtx_plain) == thrd_success)
    {
        printf("Mutex initializes successfully\n");
    }
    else
        goto error;
    // allocate new buffer
    if (monitor_set_buff_size(monitor_buff_size))
    {
        printf("Create buffer successfully\n");
    }
    else
        goto error;

success:
    monitor_enable_read(true);
    monitor_enable_write(true);
    is_init = true;
    return 0;

error:
    close(fd);
    return -1;
}

int monitor_open()
{
    monitor_close();

    //open serial port dev file
    fd = open(file, O_RDWR | O_NDELAY | O_NOCTTY | O_SYNC); //  will disable timeout setting
    if (fd > 0)
    {
        printf("Open %s successfully\n", file);
        is_open = true;
        return 0;
    }
    else
    {
        printf("Open %s failed\n", file);
        is_open = false;
        return -1;
    }
}

void monitor_close()
{
    if (is_open)
    {
        printf("Monitor is closed\n");
        monitor_enable_read(false);
        monitor_enable_write(false);
        mtx_lock(&mutex);
        is_open = false;
        mtx_unlock(&mutex);
        close(fd);
    }
}
// enable or disable read function
void monitor_enable_read(bool r)
{
    mtx_lock(&mutex);
    e_read = r;
    mtx_unlock(&mutex);
}

// enable or disable write function
void monitor_enable_write(bool w)
{
    mtx_lock(&mutex);
    e_write = w;
    mtx_unlock(&mutex);
}

int monitor_start()
{
    if (!is_init)
        goto error;

    if (!is_open)
    {
        if (monitor_open() < 0)
            goto error;
    }

    // thread is created by user
    if(is_thread_start)
    {
        printf("Monitor is starting\n");
        return 0;
    }

    // create monitor thread
    if (thrd_create(&monitor_thread, monitor_thread_func, NULL) == thrd_success)
    {
        printf("Start monitor successfully\n");
        monitor_enable_read(true); // make sure enable read in default
        monitor_enable_write(true);
        is_thread_start = true;
        return 0;
    }

error:
    printf("Failed to start monitor\n");
    monitor_close();
    return -1;
}

int monitor_thread_func(void *arg)
{
    for (;;)
    {
        mtx_lock(&mutex);
        if (!is_open || !e_read)
        {
            mtx_unlock(&mutex);
            continue;
        }
        int nbyte = read(fd, monitor_buffer, monitor_buff_size);
        if (nbyte > 0)
        {
            printf("%s", monitor_buffer);
            memset(monitor_buffer, 0, (nbyte > monitor_buff_size ? monitor_buff_size : nbyte));
        }
        mtx_unlock(&mutex);
    }

    close(fd);
    return 0;
}

int monitor_set_buff_size(size_t size)
{
    if (size == 0)
        return -1;

    if (size < MONITOR_BUFF_MIN_SIZE)
        size = MONITOR_BUFF_MIN_SIZE;

    monitor_buff_size = size;

    mtx_lock(&mutex);
    if (monitor_buffer)
        free(monitor_buffer);

    monitor_buffer = (uint8_t *)calloc(monitor_buff_size, sizeof(uint8_t));
    mtx_unlock(&mutex);
    if (monitor_buffer)
        return 1;
    else
        return -1;
}

int monitor_write(uint8_t *buffer, size_t len)
{
    mtx_lock(&mutex);
    if (!is_open || !e_write)
    {
        mtx_unlock(&mutex);
        return -1;
    }
    int nbyte = write(fd, buffer, len);
    mtx_unlock(&mutex);
    return nbyte;
}

void monitor_config(int fd, speed_t baud)
{
    struct termios options;
    /*
    * Get the current options for the port...
    */
    tcgetattr(fd, &options);
    /*
    * Set the baud rates to 19200...
    */
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);
    /*
    * Enable the receiver and set local mode...
    */
    options.c_cflag |= (CLOCAL | CREAD);
    /*
    * Set the new options for the port...
    */
    options.c_cflag &= ~PARENB; // disable parity
    options.c_cflag &= ~CSTOPB; // stop bit = 1
    options.c_cflag &= ~CSIZE;  // disable bit mask for data bit
    options.c_cflag |= CS8;     // 8 bits data bit

    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST; // Raw output is selected by resetting the OPOST
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_cc[VMIN] = 0;  // minnimum of character to read
    options.c_cc[VTIME] = 1; // wait for data in second
    // Timeouts are ignored in canonical input mode or
    // when the NDELAY option is set on the file via open or fcntl.

    tcsetattr(fd, TCSANOW, &options);
}

int monitor_read_string(uint8_t *buf, int timeout)
{
    int i = timeout;
    int bytes = -1;

    if (buf == NULL)
        return -1;

    mtx_lock(&mutex);

    if (!is_open)
        goto exit;

    while (i > 0)
    {
        usleep(1000);
        uint8_t c = 0;
        if (read(fd, &c, 1) > 0)
        {
            buf[bytes] = c;
            bytes++;
            i = timeout;

            if (c == '\n' || c == '\r')
                goto exit;
        }
        i--;
    }
exit:
    mtx_unlock(&mutex);
    return bytes;
}

speed_t get_baud_code(speed_t b)
{
    for (size_t i = 0; i < BAUD_COUNT; i++)
    {
        if (b == baud_rates[i][0])
            return baud_rates[i][1];
    }
    return B115200;
}
