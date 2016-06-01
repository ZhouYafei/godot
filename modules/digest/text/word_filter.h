/*************************************************************************/
/*  word_filter.h                                                        */
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
#ifndef __WORD_FILTER_H__
#define __WORD_FILTER_H__

#include "core/reference.h"

class WordFilter : public Reference {
	OBJ_TYPE(WordFilter, Reference);

	typedef struct TAG {

		TAG() : finish(false) {}
		bool finish;
		HashMap<uint16_t, TAG> nexts;
	} TAG;

	typedef HashMap<uint16_t, TAG> MapFilters;
	MapFilters filters;
	Set<uint16_t> symbols;

	int _check_sensitive_word(const String& p_text, int p_pos, bool p_match_maximum);

protected:
	static void _bind_methods();

public:
	WordFilter();
	~WordFilter();

	Error load(const String& p_path);
	String filter(const String& p_text, bool p_match_maximum = false, const String& p_replace = "*");
};

#endif // __WORD_FILTER_H__
