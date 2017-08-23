/*************************************************************************/
/*  register_types.h                                                     */
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
#ifdef MODULE_QRCODE_ENABLED

#include <memory.h>
#include <string.h>
#include "qrcode.h"

#include "object_type_db.h"
#include "core/globals.h"
#include "core/reference.h"
#include "core/image.h"
#include "register_types.h"

#undef MODE_NUMERIC
#undef MODE_ALPHANUMERIC
#undef MODE_BYTE

#undef ECC_LOW
#undef ECC_MEDIUM
#undef ECC_QUARTILE
#undef ECC_HIGH

class QrCode: public Reference {

	OBJ_TYPE(QrCode, Reference);

public:
	enum QrCodeFormat {
		MODE_NUMERIC = 0,
		MODE_ALPHANUMERIC = 1,
		MODE_BYTE = 2,
	};

	enum EccMode {
		ECC_LOW = 0,
		ECC_MEDIUM = 1,
		ECC_QUARTILE = 2,
		ECC_HIGH = 3,
	};

	QRCode qr;
	DVector<uint8_t> data;

protected:
	
	static void _bind_methods();

public:
	QrCode();
	virtual ~QrCode();

	int init(const String& p_url, int p_version, EccMode p_ecc);

	int get_size();
	bool get_pixel(int p_x, int p_y);
	Image get_image(Color p_fg = Color(0, 0, 0), Color p_bg = Color(1, 1, 1));
};
VARIANT_ENUM_CAST(QrCode::QrCodeFormat);
VARIANT_ENUM_CAST(QrCode::EccMode);

QrCode::QrCode() {

	memset(&qr, 0, sizeof(qr));
}

QrCode::~QrCode() {}

int QrCode::init(const String& p_url, int p_version, EccMode p_ecc) {

	ERR_EXPLAIN("Version must in range(0, 40)");
	ERR_FAIL_COND_V(p_version < 0 || p_version > 40, ERR_PARAMETER_RANGE_ERROR);

	int sz = qrcode_getBufferSize(p_version);
	data.resize(sz * sz);

	DVector<uint8_t>::Write w = data.write();

	CharString utf8 = p_url.utf8();	
	int ret = qrcode_initBytes(&qr, w.ptr(), p_version, p_ecc, (uint8_t *) utf8.get_data(), utf8.length());

	w = DVector<uint8_t>::Write();

	return ret == 0 ? OK : FAILED;
}

int QrCode::get_size() {

	return qr.size;
}

bool QrCode::get_pixel(int p_x, int p_y) {

	return qrcode_getModule(&qr, p_x, p_y);
}

Image QrCode::get_image(Color p_fg, Color p_bg) {

	int size = get_size();
	Image img(size, size, false, Image::FORMAT_RGB);

	for(int x = 0; x < size; x++) {
		for(int y = 0; y < size; y++) {

			img.put_pixel(x, y, get_pixel(x, y) ? p_fg : p_bg);
		}
	}

	return img;
}

void QrCode::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("init", "url", "version", "ecc"), &QrCode::init);
	ObjectTypeDB::bind_method(_MD("get_size"), &QrCode::get_size);
	ObjectTypeDB::bind_method(_MD("get_pixel", "x", "y"), &QrCode::get_pixel);
	ObjectTypeDB::bind_method(_MD("get_image", "fg_color", "bg_color"), &QrCode::get_image);

	BIND_CONSTANT(MODE_NUMERIC);
	BIND_CONSTANT(MODE_ALPHANUMERIC);
	BIND_CONSTANT(MODE_BYTE);

	BIND_CONSTANT(ECC_LOW);
	BIND_CONSTANT(ECC_MEDIUM);
	BIND_CONSTANT(ECC_QUARTILE);
	BIND_CONSTANT(ECC_HIGH);
}

void register_qrcode_types() {

	ObjectTypeDB::register_type<QrCode>();
}

void unregister_qrcode_types() {}

#else

void register_qrcode_types() {}
void unregister_qrcode_types() {}

#endif // MODULE_QRCODE_ENABLED
