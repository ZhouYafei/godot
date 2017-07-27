/*************************************************************************/
/*  spine.cpp                                                            */
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

//#include "method_bind_ext.inc"
#include "osprite.h"
#include "osprite_collision.h"

void OSprite::_dispose() {

	if (playing) {
		// stop first
		stop();
	}
	res = RES();
	update();
}

static int _string_find(const String& str, CharType ch) {

	for(int i = 0; i < str.size(); i++)
		if(str[i] == ch)
			return i;
	return -1;
}

void OSprite::_animation_draw() {

	if (!res.is_valid())
		return;

	if(!has(current_animation)) {

		stop();
		return;
	}

	const OSpriteResource::Action *action = action_cache;
	if(action == NULL)
		return;
	// current frame index
	int index = frame_cache;
	if(index == -1 || action == NULL)
		return;
	const OSpriteResource::Data& data = res->datas[action->index];

	Vector2 scale = Vector2(get_action_scale(), get_action_scale());

	if(action->pattern != "") {

		if(text.empty())
			return;
		const String& pattern = action->pattern;
		int total_frames = action->to - action->from + 1;
		Vector2 offset = Vector2(0, 0);

		int char_width = 0;
		for(int i = action->from; i < action->to; i++) {

			const OSpriteResource::Pool& pool = data.pools[i];
			if(pool.frame == -1)
				continue;
			const OSpriteResource::Frame& frame = res->frames[pool.frame];
			if(frame.tex.is_null())
				continue;
			if(char_width < frame.region.size.x)
				char_width = frame.region.size.x;
		}

		int valid_chars = 0;
		for(int i = 0; i < text.size(); i++) {

			CharType ch = text[i];
			int pos = _string_find(pattern, ch);
			if(pos == -1 || pos >= total_frames)
				continue;
           if(action->from + pos < 0 || action->from + pos >= data.pools.size())
                continue;
			const OSpriteResource::Pool& pool = data.pools[action->from + pos];
			if(pool.frame == -1)
				continue;
			const OSpriteResource::Frame& frame = res->frames[pool.frame];
			if(frame.tex.is_null())
				continue;
			valid_chars ++;
		}
		int text_width = char_width * valid_chars + text_space * (valid_chars - 1);

		switch(text_align) {

		case ALIGN_RIGHT:
			offset.x = -text_width;
			break;

		case ALIGN_CENTER:
			offset.x = -text_width / 2;
			break;

		case ALIGN_LEFT:
			break;
		}
		offset.x += char_width / 2;

		for(int i = 0; i < text.size(); i++) {

			CharType ch = text[i];
			int pos = _string_find(pattern, ch);
			if(pos == -1 || pos >= total_frames)
				continue;
			const OSpriteResource::Pool& pool = data.pools[action->from + pos];
			if(pool.frame == -1)
				continue;
			const OSpriteResource::Frame& frame = res->frames[pool.frame];
			if(frame.tex.is_null())
				continue;

			const Rect2& src_rect = frame.region;

			Rect2 rect = pool.rect;
			Size2 delta = ((scale - Size2(1, 1)) / 2) * rect.size;
			if(frame.rotated)
				rect.pos -= Vector2(-delta.y, delta.x);
			else
				rect.pos -= delta;
			rect.size += (delta * 2);

			this->draw_texture_rect_region(frame.tex, Rect2(rect.pos + offset, rect.size), src_rect, modulate, frame.rotated);
			offset.x += char_width + text_space;
		}
		return;
	}

	const OSpriteResource::Pool& pool = data.pools[index];
	if(pool.frame == -1 || pool.frame >= res->frames.size())
		return;
	const OSpriteResource::Frame& frame = res->frames[pool.frame];

	if(frame.tex.is_valid()) {

		const Rect2& src_rect = frame.region;

		if(pool.shadow_rect.size.x != 0 && pool.shadow_rect.size.y != 0) {

			Rect2 rect = pool.shadow_rect;
			{
				Size2 delta = ((scale - Size2(1, 1)) / 2) * rect.size;
				if(frame.rotated)
					rect.pos -= Vector2(-delta.y, delta.x);
				else
					rect.pos -= delta;
				rect.size += (delta * 2);
			}
			draw_texture_rect_region(frame.tex, pool.shadow_rect, src_rect, shadow_color, frame.rotated);
		}
		Rect2 rect = pool.rect;
		{
			Size2 delta = ((scale - Size2(1, 1)) / 2) * rect.size;
			if(frame.rotated)
				rect.pos -= Vector2(-delta.y, delta.x);
			else
				rect.pos -= delta;
			rect.size += (delta * 2);
		}
		draw_texture_rect_region(frame.tex, rect, src_rect, modulate, frame.rotated);
	}

	if(debug_collisions) {

		const OSpriteResource::Blocks& blocks = get_collisions();
		for(int i = 0; i < blocks.size(); i++) {

			const OSprite::Block& rect = blocks[i];
			static Color color = Color(0, 1, 1, 0.5);
			draw_circle(rect.pos * scale, rect.radius * scale.x, color);
		}
	}
}

void OSprite::_animation_process(float p_delta) {

	if (speed_scale == 0)
		return;
	current_pos += p_delta * speed_scale;
	if(current_pos <= delay)
		return;

	bool new_cycle = false;
	if(action_cache != NULL) {
		action_time -= p_delta * speed_scale;
		if(action_time <= 0) {
			start_trigged = false;
			end_trigged = false;
			new_cycle = true;
			action_time += (action_cache->to - action_cache->from + 1) * res->fps_delta;
			if(!loop)
				_set_process(false);
		}
	}

	if(!is_visible())
		return;

	if(action_cache != NULL) {
		if(new_cycle && !end_trigged) {

			end_trigged = true;
			//printf("animation_end %s %s\n", current_animation.utf8().get_data(), loop ? "true" : "false");
			emit_signal("animation_end", current_animation, !loop);
		}
		else if(!start_trigged) {

			start_trigged = true;
			//printf("animation_start %s %s\n", current_animation.utf8().get_data(), loop ? "true" : "false");
			emit_signal("animation_start", current_animation);
		}
	}

	frame_cache = _get_frame(action_cache);

	update();
}

void OSprite::_set_process(bool p_process, bool p_force) {

	switch (animation_process_mode) {

	case ANIMATION_PROCESS_FIXED: set_fixed_process(p_process); break;
	case ANIMATION_PROCESS_IDLE: set_process(p_process); break;
	}
}

int OSprite::_get_frame(const OSpriteResource::Action *&p_action) {

	static OSprite::OSpriteResource::Data dummy;
	if(!res.is_valid())
		return -1;

	if(!has(current_animation)) {

		ERR_EXPLAIN("Unknown animation: " + current_animation);
		ERR_FAIL_V(-1);
	}

	p_action = res->action_names[current_animation];
	ERR_FAIL_COND_V(p_action == NULL, -1);

	int total_frames = p_action->to - p_action->from + 1;
	int index = int(current_pos / res->fps_delta) % total_frames;

	if(prev_frame == -1)
		prev_frame = (forward) ? p_action->from : p_action->to;

	// make un-loop animation end at last frame
	if(!loop) {
		// prev_frame is last frame when rewind
		if(forward && (prev_frame > index))
			index = prev_frame;
		else if(!forward && (prev_frame < index))
			index = prev_frame;

	} else {
		// index always include([0] <-> [total_frames - 1])
		if(index < prev_frame && prev_frame != total_frames - 1)
			index = total_frames - 1;
		else if(prev_frame == total_frames - 1 && index != 0)
			index = 0;
		prev_frame = index;
	}
	if(forward)
		index += p_action->from;
	else
		index = p_action->to - index - 1;

	return index;
}

bool OSprite::_set(const StringName& p_name, const Variant& p_value) {

	String name = p_name;

	if (name == "playback/play") {

		String which = p_value;
		if (res.is_valid()) {

			if (which == "[stop]")
				stop();
			else if (has(which)) {
				reset();
				play(which, loop, forward);
			}
		} else
			current_animation = which;
	}
	else if (name == "playback/loop") {

		if (res.is_valid() && has(current_animation))
			play(current_animation, p_value, forward);
		else
			loop = p_value;
	}
	else if (name == "playback/forward") {

		forward = p_value;
	}
	return true;
}

bool OSprite::_get(const StringName& p_name, Variant &r_ret) const {

	String name = p_name;

	if (name == "playback/play") {

		r_ret = current_animation;
	}
	else if (name == "playback/loop")
		r_ret = loop;
	else if (name == "playback/forward")
		r_ret = forward;

	return true;
}

void OSprite::_get_property_list(List<PropertyInfo> *p_list) const {

	List<String> names;

	if (res.is_valid()) {

		for(int i = 0; i < res->actions.size(); i++)
			names.push_back(res->actions[i].name);
	}
	{
		names.sort();
		names.push_front("[stop]");
		String hint;
		for(List<String>::Element *E = names.front(); E; E = E->next()) {

			if (E != names.front())
				hint += ",";
			hint += E->get();
		}

		p_list->push_back(PropertyInfo(Variant::STRING, "playback/play", PROPERTY_HINT_ENUM, hint));
		p_list->push_back(PropertyInfo(Variant::BOOL, "playback/loop", PROPERTY_HINT_NONE));
		p_list->push_back(PropertyInfo(Variant::BOOL, "playback/forward", PROPERTY_HINT_NONE));
	}
}

void OSprite::_notification(int p_what) {

	switch (p_what) {

	case NOTIFICATION_ENTER_TREE: {

		//make sure that a previous process state was not saved
		//only process if "processing" is set
		set_fixed_process(false);
		set_process(false);
		if (active)
			_set_process(true);

		OSpriteCollision::get_singleton()->add(this, collision_mode);

	} break;
	case NOTIFICATION_READY: {

		if (active && has(current_animation)) {
			play(current_animation, loop, forward);
		}
		orig_pos = Node2D::get_pos();
		orig_rot = Node2D::get_rot();
	} break;
	case NOTIFICATION_PROCESS: {
		if (animation_process_mode == ANIMATION_PROCESS_FIXED)
			break;

		_animation_process(get_process_delta_time());
	} break;
	case NOTIFICATION_FIXED_PROCESS: {

		if (animation_process_mode == ANIMATION_PROCESS_IDLE)
			break;

		_animation_process(get_fixed_process_delta_time());
	} break;

	case NOTIFICATION_DRAW: {

		_animation_draw();
	} break;

	case NOTIFICATION_EXIT_TREE: {

		_set_process(false);
		OSpriteCollision::get_singleton()->remove(this, collision_mode);
	} break;
	}
}

void OSprite::set_resource(Ref<OSprite::OSpriteResource> p_data) {

	String anim = current_animation;
	// cleanup
	_dispose();

	res = p_data;
	if (res.is_null())
		return;

	if(has(anim))
		current_animation = anim;

	if (current_animation != "[stop]")
		play(current_animation, loop, forward);
	else
		reset();

	_change_notify();
}

Ref<OSprite::OSpriteResource> OSprite::get_resource() const {

	return res;
}

bool OSprite::has(const String& p_name) const {

	ERR_FAIL_COND_V(!res.is_valid(), false);
	return res->action_names.has(p_name);
}

bool OSprite::play(const String& p_name, bool p_loop, bool p_forward, int p_delay) {

	ERR_FAIL_COND_V(!res.is_valid(), false);
	ERR_EXPLAIN("Unknown action: " + p_name);
	ERR_FAIL_COND_V(!res->action_names.has(p_name), false);

	// loop & same animation, ignored setup variables
	if(p_loop && current_animation == p_name && p_loop == loop)
		return true;

	current_animation = p_name;
	loop = p_loop;
	forward = p_forward;
	delay = p_delay;
	current_pos = 0;
	prev_frame = -1;
	frame_cache = _get_frame(action_cache);
	action_time = (action_cache->to - action_cache->from + 1) * res->fps_delta;
	start_trigged = false;
	end_trigged = false;

	OSpriteResource::Action *action = res->action_names[p_name];
	// ignore text actions
	if(action->pattern != "")
		return true;

	frames = (action->to - action->from) + 1;

	playing = true;
	_set_process(true);

	return true;
}

void OSprite::stop() {

	_set_process(false);
	playing = false;
	current_animation = "[stop]";
	reset();
}

bool OSprite::is_playing(const String& p_name) const {

	return playing && (p_name == "" || p_name == current_animation);
}

void OSprite::set_forward(bool p_forward) {

	forward = p_forward;
}

bool OSprite::is_forward() const {

	return forward;
}

String OSprite::get_current_animation() const {

	ERR_FAIL_COND_V(!res.is_valid(), "");
	return current_animation;
}

float OSprite::get_current_animation_pos() const {

	if(!res.is_valid() || !has(current_animation))
		return 0;

	return Math::fmod(current_pos, get_current_animation_length());
}

float OSprite::get_current_animation_length() const {

	if(!res.is_valid() || !has(current_animation))
		return 0;

	return frames * res->fps_delta / speed_scale;
}

void OSprite::reset() {

	ERR_FAIL_COND(!res.is_valid());
	current_pos = 0;
	prev_frame = 0;
}

void OSprite::seek(float p_pos) {

	_animation_process(p_pos - current_pos);
}

float OSprite::tell() const {

	return current_pos;
}

void OSprite::set_speed(float p_speed) {

	speed_scale = p_speed;
}

float OSprite::get_speed() const {

	return speed_scale;
}

void OSprite::set_active(bool p_value) {

	if (active == p_value)
		return;

	active = p_value;
	_set_process(active, true);
}

bool OSprite::is_active() const {

	return active;
}

void OSprite::set_modulate(const Color& p_color) {

	modulate = p_color;
	update();
}

Color OSprite::get_modulate() const{

	return modulate;
}

void OSprite::set_shadow_color(const Color& p_color) {

	shadow_color = p_color;
	update();
}

Color OSprite::get_shadow_color() const {

	return shadow_color;
}

void OSprite::set_flip_x(bool p_flip) {

	flip_x = p_flip;
	update();
}

void OSprite::set_flip_y(bool p_flip) {

	flip_y = p_flip;
	update();
}

bool OSprite::is_flip_x() const {

	return flip_x;
}

bool OSprite::is_flip_y() const {

	return flip_y;
}

Array OSprite::get_animation_names() const {

	ERR_FAIL_COND_V(!res.is_valid(), Array());

	Array names;
	for(int i = 0; i < res->actions.size(); i++)
		names.push_back(res->actions[i].name);
	return names;
}

void OSprite::set_animation_process_mode(OSprite::AnimationProcessMode p_mode) {

	if (animation_process_mode == p_mode)
		return;

	//bool pr = processing;
	bool pr = playing;
	if (pr)
		_set_process(false);
	animation_process_mode = p_mode;
	if (pr)
		_set_process(true);
}

OSprite::AnimationProcessMode OSprite::get_animation_process_mode() const {

	return animation_process_mode;
}

void OSprite::set_collision_mode(CollisionMode p_mode) {

	CollisionMode old_mode = collision_mode;
	collision_mode = p_mode;
	OSpriteCollision::get_singleton()->changed(this, old_mode, collision_mode);
}

OSprite::CollisionMode OSprite::get_collision_mode() const {

	return collision_mode;
}

void OSprite::set_debug_collisions(bool p_enable) {

	debug_collisions = p_enable;
	update();
}

bool OSprite::is_debug_collisions() const {

	return debug_collisions;
}

void OSprite::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_resource", "res:OSpriteResource"), &OSprite::set_resource);
	ObjectTypeDB::bind_method(_MD("get_resource:OSpriteResource"), &OSprite::get_resource);

	ObjectTypeDB::bind_method(_MD("has", "name"), &OSprite::has);
	ObjectTypeDB::bind_method(_MD("play", "name", "loop", "forward", "delay"), &OSprite::play, false, true, 0);
	ObjectTypeDB::bind_method(_MD("stop"), &OSprite::stop);
	ObjectTypeDB::bind_method(_MD("is_playing", "name"), &OSprite::is_playing, String(""));
	ObjectTypeDB::bind_method(_MD("set_forward", "forward"), &OSprite::set_forward);
	ObjectTypeDB::bind_method(_MD("is_forward"), &OSprite::is_forward);
	ObjectTypeDB::bind_method(_MD("get_current_animation"), &OSprite::get_current_animation);
	ObjectTypeDB::bind_method(_MD("get_current_animation_pos"), &OSprite::get_current_animation_pos);
	ObjectTypeDB::bind_method(_MD("get_current_animation_length"), &OSprite::get_current_animation_length);
	ObjectTypeDB::bind_method(_MD("reset"), &OSprite::reset);
	ObjectTypeDB::bind_method(_MD("seek", "pos"), &OSprite::seek);
	ObjectTypeDB::bind_method(_MD("tell"), &OSprite::tell);
	ObjectTypeDB::bind_method(_MD("set_active", "active"), &OSprite::set_active);
	ObjectTypeDB::bind_method(_MD("is_active"), &OSprite::is_active);
	ObjectTypeDB::bind_method(_MD("set_speed", "speed"), &OSprite::set_speed);
	ObjectTypeDB::bind_method(_MD("get_speed"), &OSprite::get_speed);
	ObjectTypeDB::bind_method(_MD("set_modulate", "modulate"), &OSprite::set_modulate);
	ObjectTypeDB::bind_method(_MD("get_modulate"), &OSprite::get_modulate);
	ObjectTypeDB::bind_method(_MD("set_shadow_color", "shadow_color"), &OSprite::set_shadow_color);
	ObjectTypeDB::bind_method(_MD("get_shadow_color"), &OSprite::get_shadow_color);
	ObjectTypeDB::bind_method(_MD("set_flip_x", "flip"), &OSprite::set_flip_x);
	ObjectTypeDB::bind_method(_MD("is_flip_x"), &OSprite::is_flip_x);
	ObjectTypeDB::bind_method(_MD("set_flip_y", "flip"), &OSprite::set_flip_y);
	ObjectTypeDB::bind_method(_MD("is_flip_y"), &OSprite::is_flip_y);
	ObjectTypeDB::bind_method(_MD("set_animation_process_mode","mode"),&OSprite::set_animation_process_mode);
	ObjectTypeDB::bind_method(_MD("get_animation_process_mode"),&OSprite::get_animation_process_mode);
	ObjectTypeDB::bind_method(_MD("set_collision_mode","mode"),&OSprite::set_collision_mode);
	ObjectTypeDB::bind_method(_MD("get_collision_mode"),&OSprite::get_collision_mode);
	ObjectTypeDB::bind_method(_MD("get_animation_names"), &OSprite::get_animation_names);

	ObjectTypeDB::bind_method(_MD("get_collisions", "global_pos"), &OSprite::_get_collisions, false);
	ObjectTypeDB::bind_method(_MD("get_action_scale"), &OSprite::get_action_scale, false);

	ObjectTypeDB::bind_method(_MD("set_debug_collisions", "enable"), &OSprite::set_debug_collisions);
	ObjectTypeDB::bind_method(_MD("is_debug_collisions"), &OSprite::is_debug_collisions);

	ObjectTypeDB::bind_method(_MD("get_text"), &OSprite::get_text, false);
	ObjectTypeDB::bind_method(_MD("set_text", "text"), &OSprite::set_text, false);
	ObjectTypeDB::bind_method(_MD("get_text_space"), &OSprite::get_text_space, false);
	ObjectTypeDB::bind_method(_MD("set_text_space", "space"), &OSprite::set_text_space, false);
	ObjectTypeDB::bind_method(_MD("get_text_align"), &OSprite::get_text_align, false);
	ObjectTypeDB::bind_method(_MD("set_text_align", "align"), &OSprite::set_text_align, false);

	ObjectTypeDB::bind_method(_MD("set_rot_diff","diff"), &OSprite::set_rot_diff);
	ObjectTypeDB::bind_method(_MD("get_rot_diff"), &OSprite::get_rot_diff);
	ObjectTypeDB::bind_method(_MD("set_ignored_rot","enable"), &OSprite::set_ignored_rot);
	ObjectTypeDB::bind_method(_MD("is_ignored_rot"), &OSprite::is_ignored_rot);
	ObjectTypeDB::bind_method(_MD("set_pos_diff","diff"), &OSprite::set_pos_diff);
	ObjectTypeDB::bind_method(_MD("get_pos_diff"), &OSprite::get_pos_diff);
	ObjectTypeDB::bind_method(_MD("set_pos","pos"), &OSprite::set_pos);
	ObjectTypeDB::bind_method(_MD("get_pos"), &OSprite::get_pos);
	ObjectTypeDB::bind_method(_MD("set_rot","radian"), &OSprite::set_rot);
	ObjectTypeDB::bind_method(_MD("get_rot"), &OSprite::get_rot);

	ADD_PROPERTY( PropertyInfo( Variant::INT, "playback/process_mode", PROPERTY_HINT_ENUM, "Fixed,Idle"), _SCS("set_animation_process_mode"), _SCS("get_animation_process_mode"));
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "playback/speed", PROPERTY_HINT_RANGE, "-64,64,0.01"), _SCS("set_speed"), _SCS("get_speed"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playback/active"), _SCS("set_active"), _SCS("is_active"));

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "misc/show_collision"), _SCS("set_debug_collisions"), _SCS("is_debug_collisions"));
	ADD_PROPERTY( PropertyInfo( Variant::INT, "misc/collision_mode", PROPERTY_HINT_ENUM, "Ignored,Fish,Bullet"), _SCS("set_collision_mode"), _SCS("get_collision_mode"));

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "modulate"), _SCS("set_modulate"), _SCS("get_modulate"));
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "shadow_color"), _SCS("set_shadow_color"), _SCS("get_shadow_color"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_x"), _SCS("set_flip_x"), _SCS("is_flip_x"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_y"), _SCS("set_flip_y"), _SCS("is_flip_y"));
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "resource", PROPERTY_HINT_RESOURCE_TYPE, "OSpriteResource"), _SCS("set_resource"), _SCS("get_resource"));

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text/text"), _SCS("set_text"), _SCS("get_text"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "text/space"), _SCS("set_text_space"), _SCS("get_text_space"));
	ADD_PROPERTY(PropertyInfo(Variant::INT, "text/align", PROPERTY_HINT_ENUM, "Left,Center,Right"), _SCS("set_text_align"), _SCS("get_text_align"));

	ADD_SIGNAL(MethodInfo("animation_start", PropertyInfo(Variant::STRING, "action")));
	ADD_SIGNAL(MethodInfo("animation_end", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::BOOL, "finish")));
	ADD_SIGNAL(MethodInfo("collision_enter", PropertyInfo(Variant::OBJECT, "owner"), PropertyInfo(Variant::OBJECT, "body")));
	ADD_SIGNAL(MethodInfo("collision_leave", PropertyInfo(Variant::OBJECT, "owner"), PropertyInfo(Variant::OBJECT, "body")));

	BIND_CONSTANT(ANIMATION_PROCESS_FIXED);
	BIND_CONSTANT(ANIMATION_PROCESS_IDLE);

	BIND_CONSTANT(COLLISION_IGNORED);
	BIND_CONSTANT(COLLISION_FISH);
	BIND_CONSTANT(COLLISION_BULLET);

	BIND_CONSTANT(ALIGN_LEFT);
	BIND_CONSTANT(ALIGN_CENTER);
	BIND_CONSTANT(ALIGN_RIGHT);
}

Rect2 OSprite::get_item_rect() const {

	if(!res.is_valid() || !has(current_animation))
		return Node2D::get_item_rect();

	const OSpriteResource::Action *action = action_cache;
	if(action == NULL)
		return Node2D::get_item_rect();
	// current frame index
	int index = frame_cache;
	if(index == -1 || action == NULL)
		return Node2D::get_item_rect();
	const OSpriteResource::Data& data = res->datas[action->index];

	Rect2 rect = data.pools[index].rect;
	const OSpriteResource::Pool& pool = data.pools[index];
	if(pool.frame == -1)
		return Node2D::get_item_rect();
	if(res->frames[pool.frame].rotated) {

		rect.size.y = -rect.size.y;
		SWAP(rect.size.x, rect.size.y);
	}

	float scale = get_action_scale();
	if(scale != 1) {

		Vector2 s = Vector2(scale, scale);
		Size2 delta = ((s - Size2(1, 1)) / 2) * rect.size;
		rect.pos -= delta;
		rect.size += (delta * 2);
	}

	return rect;
}

const OSprite::Blocks& OSprite::get_collisions() const {

	static OSprite::Blocks empty;
	ERR_FAIL_COND_V(!res.is_valid(), empty);

	const OSpriteResource::Action *action = action_cache;
	if(action == NULL)
		return empty;
	// current frame index
	int index = frame_cache;
	if(index == -1 || action == NULL)
		return empty;

	if(action->blocks.size() > 0)
		return action->blocks;

	const OSpriteResource::Data& data = res->datas[action->index];
	if(index >= data.blocks.size())
		return empty;
	return data.blocks[index];
}

float OSprite::get_action_scale() const {

	ERR_FAIL_COND_V(!res.is_valid(), 1);
	float scale = 1;
	if(action_cache != NULL)
		scale *= action_cache->scale;
	return scale;
}

const String& OSprite::get_text() const {

	return text;
}

void OSprite::set_text(const String& p_text) {

	if(text == p_text)
		return;
	_set_process(false);

	text = p_text;
	update();
}

int OSprite::get_text_space() const {

	return text_space;
}

void OSprite::set_text_space(int p_space) {

	if(text_space == p_space)
		return;
	_set_process(false);

	text_space = p_space;
	update();
}

OSprite::TextAlign OSprite::get_text_align() const {

	return text_align;
}

void OSprite::set_text_align(OSprite::TextAlign p_align) {

	if(text_align == p_align)
		return;
	_set_process(false);

	text_align = p_align;
	update();
}

Array OSprite::_get_collisions(bool p_global_pos) const {

	const Blocks& boxes = get_collisions();
	Array result;
	result.resize(boxes.size());
	float rot = get_rot();
	float scale = get_scale().x;

	Matrix32 xform = p_global_pos ? get_global_transform() : get_transform();

	for(int i = 0; i < boxes.size(); i++) {

		const Block& box = boxes[i];
		Dictionary d;
		d["pos"] = xform.xform(box.pos);
		d["radius"] = box.radius * scale;
		result[i] = d;
	}
	return result;
}

void OSprite::set_rot_diff(real_t p_diff) {

	rot_diff = p_diff;
	set_rot(orig_rot);
}

real_t OSprite::get_rot_diff() const {

	return rot_diff;
}

void OSprite::set_ignored_rot(bool p_enable) {

	ignore_rotate = p_enable;
}

bool OSprite::is_ignored_rot() const {

	return ignore_rotate;
}

void OSprite::set_pos_diff(const Point2& p_diff) {

	pos_diff = p_diff;
	set_pos(orig_pos);
}

const Point2& OSprite::get_pos_diff() const {

	return pos_diff;
}

#ifndef TOOLS_ENABLED
void OSprite::set_pos(const Point2& p_pos) {

	orig_pos = p_pos;
	Node2D::set_pos(p_pos + pos_diff);
}

const Point2& OSprite::get_pos() const {

	return orig_pos;
}
#endif

void OSprite::set_rot(real_t p_radians) {

	if(ignore_rotate)
		return;
	orig_rot = p_radians;
	Node2D::set_rot(p_radians + rot_diff);
}

real_t OSprite::get_rot() const {

	return orig_rot;
}

OSprite::OSprite() {

	res = RES();

	speed_scale = 1;
	active = false;
	animation_process_mode = ANIMATION_PROCESS_IDLE;
	collision_mode = COLLISION_IGNORED;
	playing = false;
	forward = true;
	debug_collisions = false;
	current_animation = "[stop]";
	loop = true;

	modulate = Color(1, 1, 1, 1);
	shadow_color = Color(0, 0, 0, 0.6);
	flip_x = false;
	flip_y = false;
	current_pos = 0;
	prev_frame = 0;
	frames = 0;
	action_cache = NULL;
	frame_cache = 0;

	text = "";
	text_space = 0;
	text_align = ALIGN_CENTER;

	pos_diff = Point2(0,0);
	rot_diff = 0;
	ignore_rotate = false;
}

OSprite::~OSprite() {

	// cleanup
	_dispose();
	// remove self
	OSpriteCollision::get_singleton()->remove(this, collision_mode);
}

#endif // MODULE_OCEAN_ENABLED
