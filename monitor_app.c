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

#define W_BUFF_SIZE (1024 * 10)
#define FILE_PRE_CMD "-f"
#define BAUD_PRE_CMD "-b"
#define MONITOR_BUFFER "-a"

static char file_name[256] = {0};
static unsigned int baud = DEFAULT_BAUD;
static char *w_buffer = NULL;
static int nbyte = 0;
static int len = 0;
static size_t buffer_size = DEFAULT_MONITOR_BUFFER_SIZE;

void parse_arg(int argc, char *args[]);
char *strlwr(char *str);

int main(int argc, char *args[])
{
    parse_arg(argc, args);

    // if no file name is provided
    if (strlen(file_name) == 0)
        snprintf(file_name, sizeof(file_name), "%s", DEFAULT_MONITOR_PORT_NAME);

    monitor_set_buff_size(buffer_size);

    if (monitor_init(file_name, baud) == 0)
    {
        printf("Monitor on:\n\t- port: %s\n\t- baud rate: %d\n\t- buffer size: %d kB\n",
               file_name, baud, buffer_size/1024);
    }
    else
    {
        printf("Failed to open port: %s with baud rate: %d\n",
               file_name, baud);
        return -1;
    }

    if ((w_buffer = (char *)calloc(W_BUFF_SIZE, sizeof(char))) == NULL)
    {
        printf("Faild to allocate write buffer\n");
        return -1;
    }

    while (1)
    {
        if (nbyte = getline(&w_buffer, &len, stdin))
        {
            const int length = (nbyte > W_BUFF_SIZE ? W_BUFF_SIZE : nbyte);
            printf("Writing %d bytes to %s\n",
                   monitor_write((uint8_t *)w_buffer, length),
                   file_name);
        }
    }

    return 0;
}

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

//
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