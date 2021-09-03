/*
    Simple serial monitor, using read, wirte device method
    Command:
            ./monitor -f </dev/port_name> -b <baud_rate> -a <byte>
    e.g:    ./monitor -f /dev/ttyUSB0 -b 115200 -a 1024
    Author  : gretboxs
    Date    : 02/09/2021
*/
#include "monitor.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
//
#define BUFF_SIZE (1024 * 10)
#define FILE_PRE_CMD "-f"
#define BAUD_PRE_CMD "-b"
#define MONITOR_BUFFER "-a"
//
#define CMD_STA_MO ((int)(1 << 1))
#define CMD_CLS_MO ((int)(1 << 2))
#define CMD_R_LINE ((int)(1 << 3))
#define CMD_DIS_R ((int)(1 << 4))
#define CMD_ENA_R ((int)(1 << 5))
#define CMD_DIS_W ((int)(1 << 6))
#define CMD_ENA_W ((int)(1 << 7))
//
static char file_name[256] = {0};
static unsigned int baud = DEFAULT_BAUD;
static char *w_buffer = NULL;
static uint8_t *r_buffer = NULL;
static int nbyte = 0;
static int len = 0;
static size_t buffer_size = DEFAULT_MONITOR_BUFFER_SIZE;
static bool cancelation_token = false;
pthread_mutex_t mutex;
//
void parse_arg(int argc, char *args[]);
char *strlwr(char *str);
int monitor_check_cmd();
void *read_line_thrd_func(void *arg);
//

int main(int argc, char *args[])
{
    // read all arg value and parse to "file_name", "baud", "buffer_size"
    parse_arg(argc, args);

    // if no file name is provided
    if (strlen(file_name) == 0)
        snprintf(file_name, sizeof(file_name), "%s", DEFAULT_MONITOR_PORT_NAME);

    // set monitor buffer size
    monitor_set_buff_size(buffer_size);

    //
    if (monitor_init(file_name, baud) == 0)
    {
        printf("Monitor configurations:\n\t- port: %s\n\t- baud rate: %d\n\t- buffer size: %d kB\n",
               file_name, baud, buffer_size / 1024);
    }
    else
    {
        printf("Failed to open port: %s with baud rate: %d\n",
               file_name, baud);
        return -1;
    }

    r_buffer = (uint8_t *)calloc(BUFF_SIZE, sizeof(uint8_t));
    w_buffer = (char *)calloc(BUFF_SIZE, sizeof(char));

    if (!r_buffer || !w_buffer)
    {
        printf("Faild to allocate write buffer\n");
        return -1;
    }
    if (monitor_start() < 0)
        return -1;

    printf("Monitor is starting...\n");

    while (1)
    {
        if (nbyte = getline(&w_buffer, &len, stdin))
        {
            if (monitor_check_cmd(w_buffer) > 0)
                continue;

            const int length = (nbyte > BUFF_SIZE ? BUFF_SIZE : nbyte);
            printf("Writing %d bytes to %s\n",
                   monitor_write((uint8_t *)w_buffer, length),
                   file_name);
        }
    }

    free(w_buffer);
    free(r_buffer);
    return 0;
}

// check out additional function
int monitor_check_cmd(char *buf)
{
    int cmd = 0;
    if (!buf)
        return cmd;

    if (strstr(buf, "-m start") != NULL)
    {
        printf("cmd: start monitor\n");
        monitor_start();
        cmd |= CMD_STA_MO;
    }
    if (strstr(buf, "-m stop") != NULL)
    {
        printf("cmd: stop monitor\n");
        monitor_close();
        cmd |= CMD_CLS_MO;
    }
    if (strstr(buf, "-r ena") != NULL)
    {
        printf("cmd: enable read\n");
        monitor_enable_read(true);
        cmd |= CMD_ENA_R;
    }
    if (strstr(buf, "-r dis") != NULL)
    {
        printf("cmd: disable read\n");
        monitor_enable_read(false);
        cmd |= CMD_DIS_R;
    }
    if (strstr(buf, "-w ena") != NULL)
    {
        printf("cmd: enab;e write\n");
        monitor_enable_write(true);
        cmd |= CMD_ENA_W;
    }
    if (strstr(buf, "-w dis") != NULL)
    {
        printf("cmd: disable write\n");
        monitor_enable_write(false);
        cmd |= CMD_DIS_W;
    }
    if (strstr(buf, "-r line") != NULL)
    {
        printf("cmd: read line\n");
        char *sub_s = strstr(buf, "-r line") + 7;

        int line = 1;
        if (sub_s != NULL)
            line = atoi(sub_s);
        if (line == 0)
            line = 1;

        printf("read %d line\n", line);

        monitor_enable_read(false);

        // prepare for create new read line thread
        pthread_t read_line_thread;
        pthread_mutex_init(&mutex, NULL);
        cancelation_token = false;
        // create read line thread
        if (pthread_create(&read_line_thread, NULL, read_line_thrd_func, &line) == 0)
        {
            bool cancel = false;
            if (nbyte = getline(&w_buffer, &len, stdin))
            {
                if (strstr(w_buffer, "q") != NULL)
                    cancel = true;

                if (strstr(w_buffer, "e") != NULL)
                    cancel = true;
            }
            if (cancel)
            {
                pthread_mutex_lock(&mutex);
                cancelation_token = true;
                pthread_mutex_unlock(&mutex);
            }
        }

        cmd |= CMD_R_LINE;
    }
    return cmd;
}

// for read line function thread
void *read_line_thrd_func(void *arg)
{
    int line = (int)*((int *)arg);
    for (size_t i = 0; i < line; i++)
    {
        monitor_read_string(r_buffer, 1000);
        printf("[%#5d]:---%s\n", i + 1, (char *)r_buffer);
        memset(r_buffer, 0, BUFF_SIZE);

        pthread_mutex_lock(&mutex);
        if (cancelation_token)
            break;
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

// parse argurment of process
void parse_arg(int argc, char *args[])
{
    for (size_t i = 0; i < argc; i++)
    {
        const char *s = strlwr(args[i]);
        if (strstr(s, "-f") != NULL)
        {
            if (i + 1 >= argc)
                break;
            snprintf(file_name, sizeof(file_name), "%s", args[i + 1]);
        }

        if (strstr(s, "-b") != NULL)
        {
            if (i + 1 >= argc)
                break;
            baud = (unsigned int)atoi(args[i + 1]);
            if (baud == 0)
                baud = DEFAULT_BAUD;
        }

        if (strstr(s, "-a") != NULL)
        {
            if (i + 1 >= argc)
                break;
            buffer_size = (size_t)atoi(args[i + 1]);
            if (buffer_size == 0 || buffer_size < MONITOR_BUFF_MIN_SIZE)
                buffer_size = MONITOR_BUFF_MIN_SIZE;
        }
    }
}

// tolower case
char *strlwr(char *str)
{
    unsigned char *p = (unsigned char *)str;

    while (*p)
    {
        *p = tolower((unsigned char)*p);
        p++;
    }

    return str;
}