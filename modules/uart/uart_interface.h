/*************************************************************************/
/*  uart_interface.h                                                     */
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
#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H

#include <core/variant.h>

class UartInterface {

protected:
    typedef struct {
#ifdef _MSC_VER
        int port;
        void *handle;
#else
        char *port;
        int handle;
#endif
    } UartDevice;

    #define MAX_DEVICES 64
    UartDevice devices[MAX_DEVICES];
    int count;

    /**
     * \fn int enumerate()
     * \brief Enumerate available serial ports (Serial, USB serial, Bluetooth serial)
     * \return number of enumerated ports
     */
    virtual int enumerate() = 0;

public:
    UartInterface();
    virtual ~UartInterface();

    /**
     * \fn int get_count()
     * \brief Return the number of enumerated ports
     * \return number of enumerated ports
     */
    int get_count();

    /**
     * \fn String get_name(int p_index)
     * \brief Get port user-friendly name
     * \param[in] index port index
     * \return null terminated port name
     */
    virtual String get_name(int p_index) = 0;

    /**
     * \fn String get_internal_name(int p_index)
     * \brief Get port operating-system name
     * \param[in] index port index
     * \return null terminated port name
     */
    virtual String get_internal_name(int p_index) = 0;
    
    /**
     * \fn int find_port(const String& p_name)
     * \brief Try to find a port given its user-friendly name
     * \param[in] name port name (case sensitive)
     * \return index of found port or -1 if not enumerated
     */
    virtual int find_port(const String& p_name);

    /**
     * \fn bool open_port(int p_index, int p_baudrate)
     * \brief Try to open a port at a specific baudrate
     * \brief (No parity, single stop bit, no hardware flow control)
     * \param[in] index port index
     * \param[in] baudrate port baudrate
     * \return 1 if opened, 0 if not available
     */
    virtual bool open_port(int p_index, int p_baudrate) = 0;
    
    /**
     * \fn void close_port(int index)
     * \brief Close an opened port
     * \param[in] index port index
     */
    virtual bool close_port(int p_index) = 0;
    
    /**
     * \fn void close_all()
     * \brief Close all opened ports
     */
    void close_all();

    /**
     * \fn int write_port(int p_index, const ByteArray& p_buffer)
     * \brief Write data to the port (non-blocking)
     * \param[in] index port index
     * \param[in] buffer pointer to transmit buffer
     * \param[in] len length of transmit buffer in bytes
     * \return number of bytes transferred
     */
    virtual int write_port(int p_index, const ByteArray& p_buffer) = 0;

    /**
     * \fn int read_port(int p_index, ByteArray& p_buffer)
     * \brief Read data from the port (non-blocking)
     * \param[in] index port index
     * \param[in] buffer pointer to receive buffer
     * \param[in] len length of receive buffer in bytes
     * \return number of bytes transferred
     */
    virtual int read_port(int p_index, ByteArray& p_buffer) = 0;
};

#endif // UART_INTERFACE_H
