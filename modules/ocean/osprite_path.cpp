/*************************************************************************/
/*  osprite_path.cpp                                                     */
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
#include "osprite_path.h"
#include "osprite.h"
#include "scene/animation/tween.h"

#define PI 3.1415926535897932385f

struct OSpritePath::Stat {

	Stat()
		: group_mode(false)
		, initialized(false)
	{}
	
	Stat(bool p_group_mode)
		: group_mode(p_group_mode)
		, initialized(false)
	{}

	int get_index() const;
	Vector2 get_point_pos(int p_index);

	virtual ~Stat() {}
	virtual bool update() { return false; }

	bool group_mode;
	bool initialized;

	// ��¼��fish/sprite��instance id
	ObjectID fish_id;
	ObjectID sprite_id;
	ObjectID tween_id;
	// fish�ڵ�
	Node *fish;
	// sprite�ڵ�
	OSprite *sprite;
	// tween�ڵ�
	Tween *tween;

	// �����ȴ�ʱ�䣨��λ�룩
	float delay;
	// ����ʱ��
	float elapsed;
	// ���ٱ��ʣ��������ã�
	float speed;
	// �ƶ����ʣ�config.fps * (move_speed / config.speed)��
	float ratio;
	// ÿ֡�ĵ�������Ϣ
	Vector<int> points;
};

struct OSpritePath::GroupStat : public OSpritePath::Stat {

	GroupStat() : Stat(true) {}
	virtual bool update();

	// �Ѿ������ʼ�ζ�����delayʱ��δ��ǰ��false
	bool activated;
	// �������ĵ���������
	int center_index;
	// ���ĵ�����
	Vector2 center_pos;
	// group���ã��Ƿ������ĵ㣩
	bool center;
	// group���ã�����ʱ�Ƿ����أ�
	bool hidden;
};


int OSpritePath::Stat::get_index() const {

	float elapsed = this->elapsed - this->delay;
	// ��ȥ�ӳ�ʱ��
	if(elapsed < 0)
		elapsed = 0;
	return int(elapsed * ratio);
}

Vector2 OSpritePath::Stat::get_point_pos(int p_index) {

	int x = points[p_index * 2];
	int y = points[p_index * 2 + 1];
	// ��С100��������Ϊ0.01�ĸ��㣩����������ʽ����
	return Vector2(x, y) * Vector2(0.01, 0.01);
}

bool OSpritePath::GroupStat::update() {

	if(!initialized) {
		// ���ó�ʼλ��/����
		Vector2 pos = get_point_pos(0);
		sprite->set_pos(pos);
		// ����ĵ�
		//	�ж��Ƿ������ĵ�
		Vector2 faceto = center ? center_pos : get_point_pos(1);
		float rot = pos.angle_to_point(faceto);
		sprite->set_rot(rot);
		if(hidden)
			sprite->set_opacity(0);
		initialized = true;
	}

	// �ӳ��ж�
	if(!activated && elapsed >= delay) {

		// ����sprite
		if(!sprite->is_active()) {

			sprite->set_active(true);
			// ��λ����Ĳ���λ��
			float t = elapsed - delay;
			sprite->seek(t);
			// ����󣬻ָ���ʾ
			if(hidden) {
				// ���루fadein��
				tween->interpolate_method(sprite, "set_opacity", 0, 1, 1, Tween::TRANS_LINEAR, Tween::EASE_IN);
			}
		}
		activated = true;
	}

	int index = get_index();
	// �Ѿ�����·��
	if(index >= points.size() / 2) {

		// �������Ƴ���Ļ��
		fish->call("kill", "fadeout");
		return false;
	}

	Vector2 last_pos = sprite->get_pos();
	Vector2 pos = get_point_pos(index);
	// ���ε�λ��һ�£�������
	if(last_pos == pos)
		return true;
	sprite->set_pos(pos);

	if(center && (index < center_index)) {

		float rot = pos.angle_to_point(center_pos);
		sprite->set_rot(rot);
	} else {

		// ��������
		float rot = last_pos.angle_to_point(pos);
		float fish_rot = sprite->get_rot();
		float diff_rot = rot - fish_rot;
		float org = diff_rot;
		// �����Ƕȷ�Χ
		while(diff_rot >= PI)
			diff_rot -= PI * 2;
		while(diff_rot <= -PI)
			diff_rot += PI * 2;
		// �𲽽ӽ�Ŀ��Ƕȣ�ÿ֡ 8/1��
		//	����ת�����ʱ�������Ƚ�Բ��
		sprite->set_rot(fish_rot + diff_rot / 8);
	}
	return true;
}

OSpritePath::Stat *OSpritePath::_get_stat(Node *p_fish) {

	for(List<Stat *>::Element *E = fishs.front(); E; E = E->next()) {

		Stat *stat = E->get();
		if(stat->fish == p_fish)
			return stat;
	}
	return NULL;
}

bool OSpritePath::set_stat(Node *p_fish, const String& p_key, const Variant& p_value) {

	Stat *info = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND_V(info == NULL, false);

	ERR_EXPLAIN("Invalid stat key: " + p_key);
	if(p_key == "speed")
		info->speed = p_value;
	else
		ERR_FAIL_V(false);

	return true;
}

Variant OSpritePath::get_stat(Node *p_fish, const String& p_key) {

	Stat *stat = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND_V(stat == NULL, false);

	ERR_EXPLAIN("Invalid stat key: " + p_key);
	if(p_key == "speed")
		return stat->speed;

	ERR_FAIL_V(Variant());
}

bool OSpritePath::add_fish(const Dictionary& p_params) {

	return true;
}

bool OSpritePath::add_group_fish(const Dictionary& p_params) {

	Object *fish = p_params["fish"].operator Object *();
	Object *sprite = p_params["sprite"].operator Object *();
	Object *tween = p_params["tween"].operator Object *();

	GroupStat info;
	info.fish = fish ? fish->cast_to<Node>() : NULL;
	info.sprite = sprite ? sprite->cast_to<OSprite>() : NULL;
	info.tween = tween ? tween->cast_to<Tween>() : NULL;
	ERR_FAIL_COND_V(info.fish == NULL || info.sprite == NULL || info.tween == NULL, false);
	info.fish_id = info.fish->get_instance_ID();
	info.sprite_id = info.sprite->get_instance_ID();
	info.tween_id = info.tween->get_instance_ID();

	info.delay = p_params["delay"];
	info.elapsed = p_params["elapsed"];
	info.speed = p_params["speed"];
	info.ratio = p_params["ratio"];
	info.points = p_params["points"];
	info.activated = false;
	info.center_index = p_params["center_index"];
	info.center_pos = p_params["center_pos"];
	info.center = p_params["center"];
	info.hidden = p_params["hidden"];

	fishs.push_back(memnew(GroupStat(info)));

	return true;
}

bool OSpritePath::remove_fish(Node *p_fish) {

	for(List<Stat *>::Element *E = fishs.front(); E; E = E->next()) {

		Stat *stat = E->get();
		if(stat->fish == p_fish) {
			fishs.erase(E);
			memdelete(stat);
			return true;
		}
	}
	return false;
}

bool OSpritePath::seek(Node *p_fish, float p_pos) {

	Stat *stat = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND_V(stat == NULL, false);

	stat->elapsed = p_pos;
	return stat->update();
}

void OSpritePath::move(float p_delta) {

	for(List<Stat *>::Element *E = fishs.front(); E;) {

		Stat *stat = E->get();
		stat->elapsed += p_delta * stat->speed;

		if(
			(ObjectDB::get_instance(stat->sprite_id) == NULL) ||
			(ObjectDB::get_instance(stat->fish_id) == NULL) ||
			(ObjectDB::get_instance(stat->tween_id) == NULL) ||
			!stat->update()
		)
		{
			List<Stat *>::Element *D = E;
			E = E->next();
			fishs.erase(D);
			memdelete(stat);
			continue;
		}
		E = E->next();
	}
}

void OSpritePath::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_stat","fish","key","value"),&OSpritePath::set_stat);
	ObjectTypeDB::bind_method(_MD("get_stat","fish","key"),&OSpritePath::get_stat);

	ObjectTypeDB::bind_method(_MD("add_fish","params"),&OSpritePath::add_fish);
	ObjectTypeDB::bind_method(_MD("add_group_fish","params"),&OSpritePath::add_group_fish);
	ObjectTypeDB::bind_method(_MD("remove_fish","fish"),&OSpritePath::remove_fish);
	ObjectTypeDB::bind_method(_MD("seek","fish","pos"),&OSpritePath::seek);
	ObjectTypeDB::bind_method(_MD("move","delta"),&OSpritePath::move);
}
