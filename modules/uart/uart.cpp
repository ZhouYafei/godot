/*************************************************************************/
/*  uart.cpp                                                             */
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

#include "uart.h"
#include "os/os.h"

#ifdef _MSC_VER
#include "uart_win32.h"
#else
#include "uart_posix.h"
#endif

Uart *Uart::instance = NULL;

int Uart::find_port(const String& p_name) {

	ERR_FAIL_COND_V(impl == NULL, -1);
	return impl->find_port(p_name);
}

bool Uart::open_port(int p_index, int p_baudrate) {

	ERR_FAIL_COND_V(impl == NULL, false);
	if(buffers.find(p_index) != NULL)
		return true;

	if(impl->open_port(p_index, p_baudrate)) {
		buffers.insert(p_index, ByteArray());
	}
	return true;
}

bool Uart::close_port(int p_index) {

	ERR_FAIL_COND_V(impl == NULL, false);
	if(buffers.find(p_index) == NULL)
		return true;

	if(impl->close_port(p_index)) {
		buffers.erase(p_index);
	}
	return true;
}

#define NUM_TABLES	8
#define NUM_KEYS	24
#pragma pack(push, 1)
typedef struct {
	uint8_t sig;
	uint8_t len;

	typedef struct {
		uint8_t bytes[3];
	} KeyCode;
	KeyCode tables[NUM_TABLES];

	uint8_t check_sum;
} ArcadeInput;
#pragma pack(pop)

void Uart::_handle_input(int p_index, ByteArray& p_input) {

	// 0x80 + LEN(1) + 8 * 3Byte (LEN) + XOR(1)
	if(p_input.size() < sizeof(ArcadeInput))
		return;

	int check_sum = 0;
	for(int i = 1; i < sizeof(ArcadeInput) - 1; i++)
		check_sum = check_sum ^ p_input.get(i);

	ByteArray::Read r = p_input.read();
	ArcadeInput *ai = (ArcadeInput *) r.ptr();
	if(ai->check_sum == check_sum) {

		for(int i = 0; i < NUM_TABLES; i++) {

			uint8_t *bytes = ai->tables[i].bytes;
			int scan_code = (bytes[2] << 16) + (bytes[1] << 8) + bytes[0];
			IntArray keys;
			for(int k = 0; k < NUM_KEYS; k++) {
				if(scan_code & (1 << k)) {
					keys.append(1 << k);
				}
			}
			//if(scan_code != 0) {
			//	printf("%d %d %06x\n", p_index, i, scan_code);
			//}
			if(keys.size() > 0) {

				//printf("%d %d %s\n", p_index, i, (Variant(keys).operator String()).utf8().get_data());
				emit_signal("keypressed", p_index, i, keys);
			}
		}
	}
	if(p_input.size() > sizeof(ArcadeInput))
		p_input = p_input.subarray(sizeof(ArcadeInput), p_input.size() - 1);
	else
		p_input.resize(0);
}

int Uart::get_count() {

	ERR_FAIL_COND_V(impl == NULL, 0);
	return impl->get_count();
}
String Uart::get_name(int p_index) {

	ERR_FAIL_COND_V(impl == NULL, "");
	return impl->get_name(p_index);
}

String Uart::get_internal_name(int p_index) {

	ERR_FAIL_COND_V(impl == NULL, "");
	return impl->get_internal_name(p_index);
}

void Uart::process() {

	ERR_FAIL_COND(impl == NULL);

	ByteArray buf;
	buf.resize(512);
	for(Map<int, ByteArray>::Element *E = buffers.front(); E; E = E->next()) {

		int index = E->key();
		for(;;) {
			int sz = impl->read_port(index, buf);
			if(sz == 0)
				break;

			ByteArray& input = E->value();
			input.append_array(buf, sz);
			_handle_input(index, input);
		}
	}
}

void Uart::_bind_methods() {

	BIND_CONSTANT(KEY_BET1);
	BIND_CONSTANT(KEY_BET2);
	BIND_CONSTANT(KEY_BET3);
	BIND_CONSTANT(KEY_BET4);
	BIND_CONSTANT(KEY_BET5);
	BIND_CONSTANT(KEY_BET6);
	BIND_CONSTANT(KEY_BET7);
	BIND_CONSTANT(KEY_BET8);
	BIND_CONSTANT(KEY_BET9);
	BIND_CONSTANT(KEY_BET10);
	BIND_CONSTANT(KEY_BET11);
	BIND_CONSTANT(KEY_BET12);
	BIND_CONSTANT(KEY_BET13);
	BIND_CONSTANT(KEY_BET14);
	BIND_CONSTANT(KEY_BET15);
	BIND_CONSTANT(KEY_CIONPRE);
	BIND_CONSTANT(KEY_FUN1);
	BIND_CONSTANT(KEY_FUN2);
	BIND_CONSTANT(KEY_FUN3);
	BIND_CONSTANT(KEY_ADDX1);
	BIND_CONSTANT(KEY_DECX1);
	BIND_CONSTANT(KEY_ADDX10);
	BIND_CONSTANT(KEY_DECX10);
	BIND_CONSTANT(KEY_CIONOUT);

	ADD_SIGNAL(MethodInfo("keypressed", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::INT, "table"), PropertyInfo(Variant::ARRAY, "keys")));

	ObjectTypeDB::bind_method(_MD("get_count"), &Uart::get_count);
	ObjectTypeDB::bind_method(_MD("get_name", "name"), &Uart::get_name);
	ObjectTypeDB::bind_method(_MD("get_internal_name", "name"), &Uart::get_internal_name);

	ObjectTypeDB::bind_method(_MD("find_port", "name"), &Uart::find_port);
	ObjectTypeDB::bind_method(_MD("open_port", "index", "baudrate"), &Uart::open_port, 115200);
	ObjectTypeDB::bind_method(_MD("close_port", "index"), &Uart::close_port);
	ObjectTypeDB::bind_method(_MD("process", "index"), &Uart::process);
}

Uart *Uart::get_singleton() {

	return instance;
}

Uart::Uart() {

#ifdef _MSC_VER
	impl = memnew(UartWin32);
#else
	impl = memnew(UartPosix);
#endif
	instance = this;

	//int index = this->find_port("COM14");
	//this->open_port(index);
	//for(;;) {

	//	this->process(0);
	//	OS::get_singleton()->delay_usec(0);
	//}
}

Uart::~Uart() {

	memdelete(impl);
	instance = NULL;
}

#endif // MODULE_UART_ENABLED
