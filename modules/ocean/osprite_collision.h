/*************************************************************************/
/*  osprite_collision.h                                                  */
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
#ifdef MODULE_OCEAN_ENABLED
#ifndef OSPRITE_COLLISION_H
#define OSPRITE_COLLISION_H

#include "osprite.h"

class OSpriteCollision : public Node {
	OBJ_TYPE(OSpriteCollision, Node)

	typedef Set<size_t> CollisionIds;
	typedef HashMap<size_t, CollisionIds> CollisionMaps;
	CollisionMaps objects;

	void _set_process(bool p_mode);
	void _check_collision();
	bool _is_collision(size_t left, size_t right);
	void _on_collision_enter(size_t left, size_t right);
	void _on_collision_leave(size_t left, size_t right);

protected:
	void _notification(int p_what);

public:
	OSpriteCollision();
	virtual ~OSpriteCollision();

	void add(OSprite *sprite);
	void remove(OSprite *sprite);

	static OSpriteCollision *get_singleton();
};


#endif // OSPRITE_COLLISION_H
#endif // MODULE_OCEAN_ENABLED
