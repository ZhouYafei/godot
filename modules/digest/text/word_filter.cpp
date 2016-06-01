/*************************************************************************/
/*  word_filter.cpp                                                      */
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
#include "word_filter.h"
#include "json_asset.h"

const wchar_t SYMBOLS[] = L"¡«¡¤£¡@#£¤%¡­¡­&¡Á£¨£©¡ª¡ª+|¡¢¡¾¡¿¡º¡»£»£º¡®¡¯¡°¡±¡¶¡·£¬¡£/£¿";

Error WordFilter::load(const String& p_path) {

	JsonAsset asset;
	Error ret = asset.load(p_path);
	if(ret != OK)
		return ret;

	Variant value = asset.get_value();
	if(value.get_type() != Variant::ARRAY)
		return ERR_FILE_UNRECOGNIZED;

	Array words = value;
	for(int i = 0; i < words.size(); i++) {

		String word = words[i];
		word = word.to_upper();
		if(word.length() < 2)
			continue;
		CharType c = word[0];
		TAG* next = &filters[c];
		for(int n = 1; n < word.length(); n++) {

			CharType c = word[n];
			next = &(next->nexts[c]);
			next->finish = (n == (word.length() - 1));
		}
	}
	return OK;
}

int WordFilter::_check_sensitive_word(const String& p_text, int p_pos, bool p_match_maximum) {

	int len = 0;
	CharType c = p_text[p_pos];
	if(!filters.has(c))
		return -1;

	String debug;
	debug += c;

	TAG *nowMap = &filters[c];
	for(int i = p_pos + 1; i < p_text.length() ; i++){

		len++;
		c = p_text[i];
		debug += c;
		if(!nowMap->nexts.has(c)) {
			if(nowMap->finish)
				return len;
			if(!p_match_maximum || symbols.find(c) == NULL)
				return -1;
		} else {
			nowMap = &nowMap->nexts[c];
		}
	}
	return -1;
}

String WordFilter::filter(const String& p_text, bool p_match_maximum, const String& p_replace) {

	CharType r = p_replace.length() > 0 ? p_replace[0] : '*';

	String text = p_text;
	String upper = p_text.to_upper();

	for(int i = 0; i < text.length(); i++) {

		int length = _check_sensitive_word(upper, i, p_match_maximum);
		for(int n = 0 ; n < length; n++)
			text[i + n] = r;
	}
	return text;
}

void WordFilter::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("load","path"),&WordFilter::load);
	ObjectTypeDB::bind_method(_MD("filter:String","text","match_maximum","replace"),&WordFilter::filter,false,"*");
}

WordFilter::WordFilter() {

	for(CharType c = 0; c < 128; c++)
		symbols.insert(c);
	for(int i = 0; i < sizeof(SYMBOLS) / sizeof(SYMBOLS[0]); i++)
		symbols.insert(SYMBOLS[i]);
}

WordFilter::~WordFilter() {
}
