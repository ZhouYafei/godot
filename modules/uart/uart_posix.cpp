/*************************************************************************/
/*  uart_posix.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*                                                                       */
/*               Cross-platform serial / RS232 library                   */
/*           Original from : https://github.com/Marzac/rs232             */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifdef MODULE_UART_ENABLED
#if !(defined(_MSC_VER) || defined(_WIN32))

#include "uart_posix.h"

#define _DARWIN_C_SOURCE

#include <unistd.h>
#define __USE_MISC // For CRTSCTS
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>

#define __USE_SVID // For strdup
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if __ANDROID_API__ < 21
#define tcdrain(fd) ioctl(fd, TCSBRK, 1)
#endif

/*************************************************************************/
// Base name for COM devices 
/*************************************************************************/
#if defined(__APPLE__) && defined(__MACH__)

static const char * devBases[] = {
    "tty."
};
static int noBases = 1;

#else

static const char * devBases[] = {
    "ttyACM", "ttyUSB", "rfcomm", "ttyS"
};
static int noBases = 4;

#endif

static int _baud_flag(int p_baudrate) {

    switch(p_baudrate) {
        case 50:      return B50;
        case 110:     return B110;
        case 134:     return B134;
        case 150:     return B150;
        case 200:     return B200;
        case 300:     return B300;
        case 600:     return B600;
        case 1200:    return B1200;
        case 1800:    return B1800;
        case 2400:    return B2400;
        case 4800:    return B4800;
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 57600:   return B57600;
        case 115200:  return B115200;
        case 230400:  return B230400;
        default:      return B0;
    }
}

void UartPosix::_append_devices(const char *p_base) {

    int base_len = strlen(p_base);
    struct dirent *dp;
    // Enumerate devices
    DIR *dirp = opendir("/dev");
    if(dirp == NULL)
        return;
    while((dp = readdir(dirp)) && count < MAX_DEVICES) {

        if((strlen(dp->d_name) >= base_len) && (memcmp(p_base, dp->d_name, base_len) == 0)) {

            UartDevice& device = devices[count ++];
            device.port = strdup(dp->d_name);
            device.handle = -1;
        }
    }
    closedir(dirp);
}

int UartPosix::enumerate() {

    close_all();

    count = 0;
    for(int i = 0; i < noBases; i++)
        _append_devices(devBases[i]);
    return count;
}

String UartPosix::get_name(int p_index) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, "");
    UartDevice& device = devices[p_index];
    if(device.port == NULL)
        return "";
    return device.port;
}

String UartPosix::get_internal_name(int p_index) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, "");
    UartDevice& device = devices[p_index];
    if(device.port == NULL)
        return "";

    char name[128];
    sprintf(name, "/dev/%s", device.port);
    return name;
}

bool UartPosix::open_port(int p_index, int p_baudrate) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, false);
    UartDevice& device = devices[p_index];
    // Close if already open
    if(device.handle != -1)
        close_port(p_index);

    // Open port
    int handle = open(get_internal_name(p_index).utf8().get_data(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(handle < 0)
        return false;

    // General configuration
    struct termios config;
    memset(&config, 0, sizeof(config));
    tcgetattr(handle, &config);
    config.c_iflag &= ~(INLCR | ICRNL);
    config.c_iflag |= IGNPAR | IGNBRK;
    config.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    config.c_cflag &= ~(PARENB | PARODD | CSTOPB | CSIZE | CRTSCTS);
    config.c_cflag |= CLOCAL | CREAD | CS8;
    config.c_lflag &= ~(ICANON | ISIG | ECHO);
    int flag = _baud_flag(p_baudrate);
    cfsetospeed(&config, flag);
    cfsetispeed(&config, flag);

    // Timeouts configuration
    config.c_cc[VTIME] = 1;
    config.c_cc[VMIN]  = 0;
    //fcntl(handle, F_SETFL, FNDELAY);

    // Validate configuration
    if (tcsetattr(handle, TCSANOW, &config) < 0) {

        close(handle);
        return false;
    }
    device.handle = handle;
    return true;
}

bool UartPosix::close_port(int p_index) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, false);
    UartDevice& device = devices[p_index];
    if(device.handle == -1)
        return false;

    if(device.port != NULL) {

        free(device.port);
        device.port = NULL;
    }
    tcdrain(device.handle);
    close(device.handle);
    device.handle = -1;
    return true;
}

int UartPosix::write_port(int p_index, const ByteArray& p_buffer) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, 0);
    UartDevice& device = devices[p_index];
    if(device.handle == -1)
        return 0;

    ByteArray::Read r = p_buffer.read();
    int bytes = write(device.handle, r.ptr(), p_buffer.size());
    if(bytes < 0)
        bytes = 0;
    return bytes;
}

int UartPosix::read_port(int p_index, ByteArray& p_buffer) {

    ERR_FAIL_COND_V(p_index < 0 || p_index >= count, 0);
    UartDevice& device = devices[p_index];
    if(device.handle == -1)
        return 0;

    ByteArray::Write w = p_buffer.write();
    int bytes = read(device.handle, w.ptr(), p_buffer.size());
    if(bytes < 0)
        bytes = 0;
    return bytes;
}

UartPosix::UartPosix() {

    enumerate();
}

UartPosix::~UartPosix() {

    close_all();
}

#endif // defined(_MSC_VER) || defined(_WIN32)
#endif // MODULE_UART_ENABLED
