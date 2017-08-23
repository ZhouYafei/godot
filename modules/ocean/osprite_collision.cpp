/*************************************************************************/
/*  osprite_collision.cpp                                                */
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

#include "osprite_collision.h"
#include "osprite.h"
#include "os/os.h"
#include "scene/main/viewport.h"

static OSpriteCollision *singleton = NULL;

void OSpriteCollision::_notification(int p_what) {

	switch(p_what) {
	case NOTIFICATION_ENTER_TREE: {
	}
	break;
	case NOTIFICATION_EXIT_TREE: {
	}
	break;

	case NOTIFICATION_FIXED_PROCESS:
	case NOTIFICATION_PROCESS: {

		delta_time += get_process_delta_time();
		if(delta_time >= 0.1) {
			_check_collision();
			delta_time -= 0.1;
		}
	}
	break;
	}
}

OSpriteCollision *OSpriteCollision::get_singleton() {

	return singleton;
}

OSpriteCollision::OSpriteCollision() {

	singleton = this;
	delta_time = 0;

	collision_objects.resize(OSprite::COLLISION_MAX);

	set_name("@OSpriteCollision");
}

OSpriteCollision::~OSpriteCollision() {

	singleton = NULL;
}

void OSpriteCollision::add(OSprite *sprite, OSprite::CollisionMode p_mode) {

	OSprite::CollisionMode mode = p_mode;
	if(mode == OSprite::COLLISION_IGNORED)
		return;

	if(objects.empty())
		_set_process(true);

	size_t id = (size_t) sprite;
	objects.insert(id, CollisionIds());
	collision_objects[mode].insert(id);
}

void OSpriteCollision::remove(OSprite *sprite, OSprite::CollisionMode p_mode) {

	size_t id = (size_t) sprite;
	if(!objects.has(id))
		return;

	CollisionIds& ids = objects[id];
	for(CollisionIds::Element *E = ids.front(); E;) {

		size_t owner = E->get();
		E = E->next();
		_on_collision_leave(id, owner);
		_on_collision_leave(owner, id);
	}

	objects.erase(id);
	OSprite::CollisionMode mode = p_mode;
	collision_objects[mode].erase(id);

	if(objects.empty())
		_set_process(false);
}

static bool sCollisionFlags[OSprite::COLLISION_MAX][OSprite::COLLISION_MAX] = {
	// ignore
	{ false, false, false }, // ignore/fish/bullet
	// fish
	{ false, false, true },	 // ignore/fish/bullet
	// bullet
	{ false, true, false },  // ignore/fish/bullet
};

void OSpriteCollision::changed(OSprite *sprite, OSprite::CollisionMode p_oldmode, OSprite::CollisionMode p_newmode) {

	size_t id = (size_t) sprite;
	if(sprite->get_collision_mode() == OSprite::COLLISION_IGNORED) {

		if(objects.has(id)) 
			remove(sprite, p_oldmode);
	} else {

		if(!objects.has(id))
			add(sprite, p_newmode);
		else {

			CollisionIds& ids = objects[id];
			for(CollisionIds::Element *E = ids.front(); E; E = E->next()) {

				size_t right = E->get();
				OSprite *c = (OSprite *) right;
				// still collision, ignore
				if(sCollisionFlags[p_newmode][c->get_collision_mode()])
					continue;
				// remove ignore-collision objects
				_on_collision_leave(id, right);
				_on_collision_leave(right, id);
			}
			collision_objects[p_oldmode].erase(id);
			collision_objects[p_newmode].insert(id);
		}
	}
}

void OSpriteCollision::_set_process(bool p_mode) {

	SceneTree *sml = dynamic_cast<SceneTree *> (OS::get_singleton()->get_main_loop());
	ERR_FAIL_COND(sml == NULL);
	Viewport *root = sml->get_root();
	ERR_FAIL_COND(root == NULL);

	if(p_mode) {

		if(!is_inside_tree())
			root->call_deferred("add_child", this);
		set_fixed_process(true);
	} else {

		if(is_inside_tree())
			root->call_deferred("remove_child", this);
		set_fixed_process(false);
	}
}

bool OSpriteCollision::_is_collision(size_t left, size_t right) {

	OSprite *owner = (OSprite *) left;
	OSprite *body = (OSprite *) right;

	if(!sCollisionFlags[owner->get_collision_mode()][body->get_collision_mode()])
		return false;

	const OSprite::Blocks& owner_boxes = owner->get_collisions();
	if(owner_boxes.size() == 0)
		return false;

	const OSprite::Blocks& body_boxes = body->get_collisions();
	if(body_boxes.size() == 0)
		return false;

	Matrix32 owner_mat = owner->get_global_transform();
	float owner_scale = owner_mat.get_scale().x * owner->get_action_scale();

	Matrix32 body_mat = body->get_global_transform();
	float body_scale = body_mat.get_scale().x * body->get_action_scale();

	for(int i = 0; i < owner_boxes.size(); i++) {

		const OSprite::Block& owner_box = owner_boxes[i];
		Vector2 from = owner_mat.xform(owner_box.pos);

		for(int j = 0; j < body_boxes.size(); j++) {

			const OSprite::Block& body_box = body_boxes[j];
			Vector2 to = body_mat.xform(body_box.pos);

			int radius = owner_box.radius * owner_scale + body_box.radius * body_scale;
			float dist = from.distance_to(to);
			if(dist <= radius)
				return true;
		}
	}
	return false;
}

void OSpriteCollision::_on_collision_enter(size_t left, size_t right) {

	OSprite *owner = (OSprite *) left;
	OSprite *body = (OSprite *) right;

	if(!objects[left].has(right)) {

		objects[left].insert(right);
		owner->emit_signal("collision_enter", owner, body);
		//printf("collision_enter %d %d\n", left, right);
	}
	if(!objects[right].has(left)) {

		objects[right].insert(left);
		body->emit_signal("collision_enter", body, owner);
		//printf("collision_enter %d %d\n", right, left);
	}
}

void OSpriteCollision::_on_collision_leave(size_t left, size_t right) {

	OSprite *owner = (OSprite *) left;
	OSprite *body = (OSprite *) right;

	if(objects[left].has(right)) {

		objects[left].erase(right);
		owner->emit_signal("collision_leave", owner, body);
		//printf("colliison_leave %d %d\n", left, right);
	}
	if(objects[right].has(left)) {

		objects[right].erase(left);
		body->emit_signal("collision_leave", body, owner);
		//printf("collision_leave %d %d\n", right, left);
	}
}

void OSpriteCollision::_check_collision() {

	CollisionIds& bullets = collision_objects[OSprite::COLLISION_BULLET];
	CollisionIds& fishs = collision_objects[OSprite::COLLISION_FISH];
	for(CollisionIds::Element *B = bullets.front(); B; B = B->next()) {

		size_t bullet = B->get();
		if(!((OSprite *) bullet)->is_inside_tree())
			continue;

		for(CollisionIds::Element *F = fishs.front(); F;) {

			size_t fish = F->get();
			if(!((OSprite *) fish)->is_inside_tree())
				continue;
			F = F->next();

			if(_is_collision(bullet, fish)) {
				_on_collision_enter(bullet, fish);
				_on_collision_enter(fish, bullet);
			} else {
				_on_collision_leave(bullet, fish);
				_on_collision_leave(fish, bullet);
			}
		}
	}
}

#endif // MODULE_OCEAN_ENABLED
