/*************************************************************************/
/*  uart.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
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
#ifndef UART_H
#define UART_H

#include "core/object.h"
#include "core/set.h"

class UartInterface;

class Uart : public Object {

	OBJ_TYPE(Uart, Object);

	int handler;
	String callback;

	UartInterface *impl;

	Map<int, ByteArray> buffers;

	static Uart* instance;

protected:

	void _handle_input(int p_index, ByteArray& p_input);

	static void _bind_methods();

public:
	enum KeyCodes {
		KEY_BET1 = 0x000080,	//押分1				16
		KEY_BET2 = 0X000040,	//押分2				17
		KEY_BET3 = 0X000020,	//押分3				18
		KEY_BET4 = 0X000010,	//押分4				19
		KEY_BET5 = 0X000008,	//押分5				20
		KEY_BET6 = 0X000004,	//押分6				21
		KEY_BET7 = 0X000002,	//押分7				22
		KEY_BET8 = 0X000001,	//押分8				23
		KEY_BET9 = 0X008000,	//押分9				8
		KEY_BET10 = 0X004000,	//押分10			9
		KEY_BET11 = 0X002000,	//押分11			10
		KEY_BET12 = 0X001000,	//押分12			11
		KEY_BET13 = 0X000800,	//押分13			12
		KEY_BET14 = 0X000400,	//押分14			13
		KEY_BET15 = 0X000200,	//押分15			14
		KEY_CIONPRE = 0X000100,	//退币按键			15
		KEY_FUN1 = 0X800000,	//功能1(清除)		0
		KEY_FUN2 = 0X400000,	//功能2(续押/取消)	1
		KEY_FUN3 = 0X200000,	//功能3(游戏设置)	2
		KEY_ADDX1 = 0X100000,	//投币或上小分		3
		KEY_DECX1 = 0X020000,	//下分X10			6
		KEY_ADDX10 = 0X040000,	//上分X100			5
		KEY_DECX10 = 0X010000,	//下分X10			7
		KEY_CIONOUT = 0X080000,	//退币信号			4
	};

	static Uart* get_singleton();

    int get_count();
    String get_name(int p_index);
    String get_internal_name(int p_index);

	int find_port(const String& p_name);
	bool open_port(int p_index, int p_baudrate = 115200);
	bool close_port(int p_index);

	void process();

	Uart();
	virtual ~Uart();
};

VARIANT_ENUM_CAST(Uart::KeyCodes);

#endif // UART_H
#endif // MODULE_UART_ENABLED
