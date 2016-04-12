/*************************************************************************/
/*  sproto.cpp                                                          */
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
#include "sproto.h"
extern "C" {
#include "sproto/sproto.h"
};

struct decode_ud {
	decode_ud() : array_tag(NULL) {}
	const char *array_tag;
	Variant result;
	Variant array;
	Variant key;
	int deep;
	int mainindex_tag;
	bool use_default;
	Sproto* sproto;
};

static int decode_callback(const struct sproto_arg *args) {

	struct decode_ud * self = (struct decode_ud*) args->ud;
	Variant& result = self->result;
	Variant& array = self->array;
	Variant value;

	if (args->index != 0) {
		// It's array
		if (args->tagname != self->array_tag) {
			self->array_tag = args->tagname;
			Dictionary object = result.operator Dictionary();
			if(!object.has(args->tagname)) {
				if(args->mainindex >= 0)
					array = Dictionary(true);
				else
					array = Array(true);
				object[args->tagname] = array;
			} else {
				array = object[args->tagname];
			}
			if (args->index < 0) {
				// It's a empty array, return now.
				return 0;
			}
		}
	}
	switch (args->type) {
	case SPROTO_TINTEGER: {
		// notice: in lua 5.2, 52bit integer support (not 64)
		value = (*(int *)args->value);
		break;
	}
	case SPROTO_TREAL: {
		value = *(double *)args->value;
		break;
	}
	case SPROTO_TBOOLEAN: {
		value = (bool) (*(unsigned long *)args->value);
		break;
	}
	case SPROTO_TSTRING: {
		String s;
		s.parse_utf8((const char *) args->value, args->length);
		value = s;
		break;
	}
	case SPROTO_TVARIANT: {
		char *buf = (char *) args->value;
		int type = *buf++;
		switch(type) {
		case SPROTO_TREAL:
			value = *(real_t*) buf;
			break;
		case SPROTO_TSTRING: {
			String s;
			s.parse_utf8((const char *) buf, args->length);
			value = s;
		}
			break;
		case SPROTO_TBOOLEAN:
			value = *(bool*) buf;
			break;
		default:
			ERR_EXPLAIN("Unknown variant type: " + String::num(type));
			ERR_FAIL_V(0);
		}
		break;
	}
	case SPROTO_TSTRUCT: {
		struct decode_ud sub;
		sub.use_default = self->use_default;
		sub.sproto = self->sproto;
		int r;

		if(args->index > 0) {
			if(self->use_default)
				value = self->sproto->get_default(sproto_name(args->subtype));
			else
				value = Dictionary(true);
		} else {
			Dictionary object = self->result.operator Dictionary();
			if(object.has(args->tagname))
				value = object[args->tagname];
			else if(self->use_default)
				value = self->sproto->get_default(sproto_name(args->subtype));
			else
				value = Dictionary(true);
		}

		sub.result = value;
		sub.deep = self->deep + 1;
		if (args->mainindex >= 0) {
			// This struct will set into a map, so mark the main index tag.
			sub.mainindex_tag = args->mainindex;
			r = sproto_decode(args->subtype, args->value, args->length, decode_callback, &sub);
			if (r < 0)
				return SPROTO_CB_ERROR;
			if (r != args->length)
			// assert(args->index > 0);
			ERR_FAIL_COND_V(sub.key.get_type() == Variant::NIL, 0);
			self->array.set(sub.key, value);
			return 0;
		} else {
			sub.mainindex_tag = -1;
			r = sproto_decode(args->subtype, args->value, args->length, decode_callback, &sub);
			if (r < 0)
				return SPROTO_CB_ERROR;
			if (r != args->length)
				return r;
			break;
		}
	}
	default:
		ERR_EXPLAIN("Invalid type");
		ERR_FAIL_V(0);
		break;
	}
	if (args->index > 0) {
		Array object = self->array.operator Array();
		object.append(value);
	} else {
		if (self->mainindex_tag == args->tagid) {
			// This tag is marked, save the value to key_index
			// assert(self->key_index > 0);
			self->key = value;
		}
		Dictionary object = self->result.operator Dictionary();
		object[args->tagname] = value;
	}
	return 0;
}

Array Sproto::_decode(struct sproto_type *p_st, const String& p_type, const ByteArray& p_stream, bool p_use_default) {

	Dictionary o(true);
	if(p_use_default)
		o = get_default(p_type);
	struct decode_ud self;
	self.use_default = p_use_default;
	self.sproto = this;
	self.result = o;
	self.deep = 0;
	self.mainindex_tag = -1;

	ByteArray::Read r = p_stream.read();

	int rt = sproto_decode(p_st, r.ptr(), p_stream.size(), decode_callback, &self);
	ERR_EXPLAIN("decode error");
	ERR_FAIL_COND_V(rt < 0, Variant());

	Array a(true);
	a.push_back(rt);
	a.push_back(o);
	return a;
}

Array Sproto::decode(const String& p_type, const ByteArray& p_stream, bool p_use_default) {

	ERR_FAIL_COND_V(proto == NULL, Variant());
	struct sproto_type *st = sproto_type(proto, p_type.utf8().get_data());
	ERR_FAIL_COND_V(st == NULL, Variant());
	return _decode(st, p_type, p_stream, p_use_default);
}

Array Sproto::proto_decode(const String& p_type, Proto p_what, const ByteArray& p_stream, bool p_use_default) {

	ERR_FAIL_COND_V(proto == NULL, Variant());
	int tag = sproto_prototag(proto, p_type.utf8().get_data());
	ERR_FAIL_COND_V(tag == -1, Variant());
	struct sproto_type *st = sproto_protoquery(proto, tag, p_what);
	ERR_FAIL_COND_V(st == NULL, Variant());
	return _decode(st, p_type, p_stream, p_use_default);
}
