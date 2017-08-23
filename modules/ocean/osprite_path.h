/*************************************************************************/
/*  osprite_path.h                                                       */
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
#ifndef OSPRITE_PATH_H
#define OSPRITE_PATH_H

#include "core/reference.h"

class OSpritePath : public Object {

	OBJ_TYPE(OSpritePath,Object);

	struct Stat;
	List<Stat *> fishs;
	Vector2 scale;

	struct GroupStat;
	struct FishStat;

	Stat *_get_stat(Node *p_fish) const;

protected:

	static void _bind_methods();

public:

	bool set_stat(Node *p_fish, const String& p_key, const Variant& p_value);
	Variant get_stat(Node *p_fish, const String& p_key);
	real_t get_length(Node *p_fish) const;

	bool add_fish(const Dictionary& p_params);
	bool add_group_fish(const Dictionary& p_params);
	bool remove_fish(Node *p_fish);
	bool seek(Node *p_fish, float p_pos);

	void move(float p_delta);
	void clear();

	void set_scale(const Vector2& p_scale);
	Vector2 get_scale() const;

	OSpritePath() {};
	virtual ~OSpritePath();
};


#endif // OSPRITE_PATH_H
