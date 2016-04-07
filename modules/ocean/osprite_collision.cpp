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

		_check_collision();
	}
	break;
	}
}

OSpriteCollision *OSpriteCollision::get_singleton() {

	return singleton;
}

OSpriteCollision::OSpriteCollision() {

	singleton = this;

	set_name("@OSpriteCollision");
}

OSpriteCollision::~OSpriteCollision() {

	singleton = NULL;
}

void OSpriteCollision::add(OSprite *sprite) {


	if(objects.empty())
		_set_process(true);

	size_t id = (size_t) sprite;
	objects.set(id, CollisionIds());
}

void OSpriteCollision::remove(OSprite *sprite) {

	size_t id = (size_t) sprite;

	CollisionIds& ids = objects[id];
	for(CollisionIds::Element *E = ids.front(); E;) {

		size_t owner = E->get();
		E = E->next();
		_on_collision_leave(id, owner);
		_on_collision_leave(owner, id);
	}

	objects.erase(id);

	if(objects.empty())
		_set_process(false);
}

void OSpriteCollision::_set_process(bool p_mode) {

	SceneTree *sml = dynamic_cast<SceneTree *> (OS::get_singleton()->get_main_loop());
	ERR_FAIL_COND(sml == NULL);
	Viewport *root = sml->get_root();
	ERR_FAIL_COND(root == NULL);

	if(p_mode) {

		if(is_inside_tree())
			return;
		root->call_deferred("add_child", this);
		set_fixed_process(true);
	} else {

		root->remove_child(this);
		set_fixed_process(false);
	}
}

static bool sCollisionFlags[OSprite::COLLISION_MAX][OSprite::COLLISION_MAX] = {
	// ignore
	{ false, false, false }, // ignore/fish/bullet
	// fish
	{ false, false, true },	 // ignore/fish/bullet
	// bullet
	{ false, true, false },  // ignore/fish/bullet
};

bool OSpriteCollision::_is_collision(size_t left, size_t right) {

	OSprite *owner = (OSprite *) left;
	OSprite *body = (OSprite *) right;

	if(!sCollisionFlags[owner->get_collision_mode()][body->get_collision_mode()])
		return false;

	const Vector<OSprite::Box>& owner_boxes = owner->get_collisions();
	if(owner_boxes.size() == 0)
		return false;

	const Vector<OSprite::Box>& body_boxes = body->get_collisions();
	if(body_boxes.size() == 0)
		return false;

	Vector2 owner_pos = owner->get_global_pos();
	float owner_rot = owner->get_rot();
	float owner_scale = owner->get_resource_scale();

	Vector2 body_pos = body->get_global_pos();
	float body_rot = body->get_rot();
	float body_scale = body->get_resource_scale();

	for(int i = 0; i < owner_boxes.size(); i++) {

		const OSprite::Box& owner_box = owner_boxes[i];
		Vector2 from = owner_pos + owner_box.pos.rotated(owner_rot);

		for(int j = 0; j < body_boxes.size(); j++) {

			const OSprite::Box& body_box = body_boxes[j];
			Vector2 to = body_pos + body_box.pos.rotated(body_rot);

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

	List<size_t> owners;
	objects.get_key_list(&owners);
	for(List<size_t>::Element *E = owners.front(); E; E = E->next()) {

		size_t owner = E->get();
		CollisionIds& ids = objects[owner];
		for(CollisionIds::Element *IE = ids.front(); IE;) {

			size_t body = IE->get();
			IE = IE->next();
			if(!_is_collision(owner, body)) {

				// body leave
				_on_collision_leave(owner, body);
				_on_collision_leave(body, owner);
			}
		}

		for(List<size_t>::Element *E2 = E->next(); E2; E2 = E2->next()) {

			size_t body = E2->get();
			if(_is_collision(owner, body)) {

				// body enter
				_on_collision_enter(owner, body);
				_on_collision_enter(body, owner);
			}
		}
	}
}

#endif // MODULE_OCEAN_ENABLED
