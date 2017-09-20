/*************************************************************************/
/*  uart_win32.cpp                                                       */
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
#if defined(_MSC_VER) || defined(_WIN32)

#include <stdio.h>
#include <string.h>

#include "uart_win32.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const char *_errno_to_string(uint32_t p_errno)    
{    
    HLOCAL LocalAddress=NULL;    
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,  
        NULL,p_errno,0,(PTSTR)&LocalAddress,0,NULL);    
    return (LPSTR)LocalAddress;    
}  

#define COM_PATTERN					"COM???"

static const char *find_pattern(const char *p_input, const char *p_pattern, int& p_value) {

    char c, n = 0;
    const char * sp = p_input;
    const char * pp = p_pattern;
	// Check for the string pattern
	for(;;) {
		c = *sp ++;
        if (c == '\0') {
            if (*pp == '?')
            	break;
            if (*sp == '\0')
            	break;
            n = 0;
            pp = p_pattern;
        } else {
            if (*pp == '?') {
         	   // Expect a digit
                if (c >= '0' && c <= '9') {
                    n = n * 10 + (c - '0');
                    if (*pp ++ == '\0')
                    	break;
                } else {
                    n = 0;
                    pp = p_pattern;
                }
            } else {
            	// Expect a character
                if (c == *pp) {
                    if (*pp ++ == '\0')
                    	break;
                } else {
                    n = 0;
                    pp = p_pattern;
                }
            }
        }
	}
	// Return the value
	p_value = n;
	return sp;
}

int UartWin32::enumerate() {

    close_all();

	// Get devices information text
	size_t size = 8192;
	char *list = (char *) malloc(size);

    SetLastError(0);
    QueryDosDeviceA(NULL, list, size);
    while(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

    	size *= 2;
    	char *tmp = (char *) realloc(list, size);
    	if(tmp == NULL) {

    		free(list);
    		return 0;
    	}
    	list = tmp;
    	SetLastError(0);
    	QueryDosDeviceA(NULL, list, size);
    }
	count = 0;
	// Gather all COM ports
    int port;
	const char *walk = find_pattern(list, COM_PATTERN, port);

	while(port > 0 && count < MAX_DEVICES) {

		UartDevice& device = devices[count ++];
		device.port = port;
		device.handle = NULL;
        walk = find_pattern(walk, COM_PATTERN, port);
    }
	free(list);
    return count;
}

String UartWin32::get_name(int p_index) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, "");
	char name[32];
	sprintf(name, "COM%i", devices[p_index].port);
    return name;
}

String UartWin32::get_internal_name(int p_index) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, "");
	char name[32];
	sprintf(name, "//./COM%i", devices[p_index].port);
    return name;
}

bool UartWin32::open_port(int p_index, int p_baudrate) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, false);
	UartDevice& device = devices[p_index];
	// Close if already open
	if(device.handle != NULL)
		close_port(p_index);
	
    DCB config;
    COMMTIMEOUTS timeouts;
	// Open COM port
	void *handle = CreateFileA(get_internal_name(p_index).utf8().get_data(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(handle == INVALID_HANDLE_VALUE) {

		printf("CreateFileA : %s\n", _errno_to_string(GetLastError()));
		return false;
	}

	// Prepare serial communication format
    memset(&config, 0, sizeof(config));
    config.DCBlength		= sizeof(config);
    config.BaudRate			= p_baudrate;
    config.ByteSize			= 8;
    config.Parity			= NOPARITY;
    config.StopBits			= ONESTOPBIT;
    config.fOutX			= 0;
    config.fInX				= 0;
    config.fDtrControl		= DTR_CONTROL_DISABLE;
    config.fRtsControl		= RTS_CONTROL_DISABLE;
    config.fOutxCtsFlow		= 0;
    config.fOutxDsrFlow		= 0;
	config.fDsrSensitivity	= 0;

	// Set the port state
    if (SetCommState(handle, &config) == 0) {

		printf("SetCommState : %s\n", _errno_to_string(GetLastError()));
        CloseHandle(handle);
        return false;
    }

	// Set read / write buffer size
	SetupComm(handle, 512, 512);

	// Prepare read / write timeouts
	timeouts.ReadIntervalTimeout         = 10;
	timeouts.ReadTotalTimeoutMultiplier  = 10;
	timeouts.ReadTotalTimeoutConstant    = 10;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant	 = 10;
	SetCommTimeouts(handle, &timeouts);

	PurgeComm(handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    device.handle = handle;
    return true;
}

bool UartWin32::close_port(int p_index) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, false);
	UartDevice& device = devices[p_index];
	if(device.handle == NULL)
		return false;
	CloseHandle(device.handle);
	device.handle = NULL;
	return true;
}

int UartWin32::write_port(int p_index, const ByteArray& p_buffer) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, 0);
	UartDevice& device = devices[p_index];
	ERR_EXPLAIN("Can not write an closed port");
	ERR_FAIL_COND_V(device.handle == NULL, 0);

	PurgeComm(device.handle, PURGE_TXCLEAR);

	uint32_t bytes = 0;
	ByteArray::Read r = p_buffer.read();
	WriteFile(device.handle, r.ptr(), p_buffer.size(), (LPDWORD) &bytes, NULL);
	return bytes;
}

int UartWin32::read_port(int p_index, ByteArray& p_buffer) {

	ERR_FAIL_COND_V(p_index < 0 || p_index >= count, 0);
	UartDevice& device = devices[p_index];
	ERR_EXPLAIN("Can not write an closed port");
	ERR_FAIL_COND_V(device.handle == NULL, 0);

	PurgeComm(device.handle, PURGE_RXCLEAR);

	uint32_t bytes = 0;
    ByteArray::Write w = p_buffer.write();
    ReadFile(device.handle, w.ptr(), p_buffer.size(), (LPDWORD) &bytes, NULL);
    return bytes;
}

UartWin32::UartWin32() {

	enumerate();
}

UartWin32::~UartWin32() {

	close_all();
}

#endif // defined(_MSC_VER) || defined(_WIN32)
#endif // MODULE_UART_ENABLED

