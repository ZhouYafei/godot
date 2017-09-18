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

	Stat();
	
	virtual ~Stat() {}

	virtual int get_index() const;
	virtual Vector2 get_point_pos(int p_index, const Vector2& p_scale);

	virtual bool init(const OSpritePath *p_path) { return false; }
	virtual bool update(const OSpritePath *p_path);

	real_t get_elapsed() const;
	void add_freeze_time(real_t p_start, real_t p_duration);

	// ��¼��fish/sprite��instance id
	ObjectID fish_id;
	ObjectID sprite_id;
	// fish�ڵ�
	Node *fish;
	// sprite�ڵ�
	OSprite *sprite;

	// �Ѿ������ʼ�ζ�����delayʱ��δ��ǰ��false
	bool activated;
	// ����ʱ�Ƿ����أ�activate�󽥱���ʾ��
	bool hidden;
	// �����ȴ�ʱ�䣨��λ�룩
	real_t delay;
	// ����ʱ��
	real_t elapsed;
	// ���ٱ��ʣ��������ã�
	real_t speed;
	// �ƶ����ʣ�config.fps * (move_speed / config.speed)��
	real_t ratio;
	// ÿ֡�ĵ�������Ϣ
	DVector<int> points;
	// ����Ч������
	typedef struct {
		// ������ʼʱ��(��)
		real_t start;
		// ��������ʱ��(��)
		real_t end;
	} Freeze;
	List<Freeze> freezes;
};

OSpritePath::Stat::Stat()
	: activated(false)
{
}

void OSpritePath::Stat::add_freeze_time(real_t p_start, real_t p_duration) {

	real_t end = p_start + p_duration;
	for(List<Freeze>::Element *E = freezes.front(); E; E = E->next()) {

		Freeze& info = E->get();
		if(end < info.start || p_start > info.end)
			continue;
		info.start = MIN(info.start, p_start);
		info.end = MAX(info.end, end);
		return;
	}

	Freeze freeze;
	freeze.start = p_start;
	freeze.end = end;
	freezes.push_back(freeze);
}

real_t OSpritePath::Stat::get_elapsed() const {

	real_t elapsed = this->elapsed;

	real_t skipped = 0;
	bool freezed = false;
	for(const List<Freeze>::Element *E = freezes.front(); E; E = E->next()) {

		const Freeze& info = E->get();
		if((elapsed + skipped) >= info.start) {

			if((elapsed + skipped) <= info.end) {

				elapsed = info.start - skipped;
				freezed = true;
				break;			
			} else {

				skipped += (info.end - info.start);
				elapsed -= (info.end - info.start);
			}
		}
	}

	// ��������ˣ�ֹͣ����
	this->sprite->set_active(!freezed);

	return elapsed - this->delay;
}

int OSpritePath::Stat::get_index() const {

	float elapsed = this->get_elapsed();
	// ��ȥ�ӳ�ʱ��
	if(elapsed < 0)
		elapsed = 0;
	return int(elapsed * ratio);
}

Vector2 OSpritePath::Stat::get_point_pos(int p_index, const Vector2& p_scale) {

	int x = points[p_index * 2];
	int y = points[p_index * 2 + 1];
	// ��С100��������Ϊ0.01�ĸ��㣩����������ʽ����
	return Vector2(x, y) * Vector2(0.01, 0.01) * p_scale;
}

bool OSpritePath::Stat::update(const OSpritePath *p_path) {

	real_t elapsed = this->get_elapsed();
	// �ӳ��ж�
	if(!activated && elapsed >= 0) {

		// ����sprite
		if(!sprite->is_active()) {

			sprite->set_active(true);
			// ��λ����Ĳ���λ��
			float t = elapsed;
			sprite->seek(t);
		}
		float opacity = sprite->get_opacity();
		// ����󣬻ָ���ʾ
		if(hidden && opacity < 1) {
			// ���루fadein��
			//	����2�루ÿ��60fps��
			opacity += (1.0 / 120);
			if(opacity > 1)
				opacity = 1;
			sprite->set_opacity(opacity);
		} else {
			activated = true;
		}
	}
	return true;
}

struct OSpritePath::FishStat : public OSpritePath::Stat {

	virtual int get_index() const;
	virtual Vector2 get_point_pos(int p_index, const Vector2& p_scale);

	virtual bool init(const OSpritePath *p_path);
	virtual bool update(const OSpritePath *p_path);

	// ·���������λ��/��ת/��ת �ȣ�
	Matrix32 mat;
	// ����/�����ζ�
	bool forward;

	typedef struct Tween {
		float delta;
		float weight;

		struct Phase {
			float delta;
			float weight;
			int trans, eases;
		};
		Vector<Phase> phases;
	} Tween;
	Tween tween;
};

int OSpritePath::FishStat::get_index() const {

	int index = Stat::get_index();
	if(index == 0 || tween.delta == 0)
		return index;
	// ����ʱ��Ƭ��ֵ������/�����ζ�ģ�⣩����Ŀ��/�ڹ� ��

	float elapsed_time = this->get_elapsed();
	// t �ǵ�ǰphase tween������ʱ��
	// b �ǲ�ֵ����ʼֵ
	// c �ǲ�ֵ������ֵ��b+c=����ֵ��
	// d �ǵ�ǰphase �ܹ�������ʱ��
	float t = Math::fmod(elapsed_time, tween.delta);
	float b = int(elapsed_time / tween.delta) * tween.delta;

	for(int i = 0; i < tween.phases.size(); i++) {

		const Tween::Phase& phase = tween.phases[i];
		if(t > phase.delta) {
			t -= phase.delta;
			b += phase.delta;
			continue;
		}
		float d = phase.delta;
		float c = d;
		elapsed_time = ::Tween::run_equation((::Tween::TransitionType) phase.trans, (::Tween::EaseType) phase.eases, t, b, c, d);
		break;
	}
	// �ƶ����� Ȩ��
	t = Math::fmod(elapsed_time, tween.delta);
	for(int i = 0; i < tween.phases.size(); i++) {

		const Tween::Phase& phase = tween.phases[i];
		if(t <= 0)
			break;
		float weight = phase.weight / tween.weight;
		float dweight = tween.delta * weight;
		float rate = dweight / phase.delta;
		if(t >= phase.delta)
			elapsed_time += (phase.delta * (rate - 1));
		else
			elapsed_time += (t * (rate - 1));
		t -= phase.delta;
	}
	return int(elapsed_time * ratio);
}

Vector2 OSpritePath::FishStat::get_point_pos(int p_index, const Vector2& p_scale) {

	return mat.xform(Stat::get_point_pos(p_index, p_scale));
}

bool OSpritePath::FishStat::init(const OSpritePath *p_path) {

	// ���ó�ʼλ��/����
	Vector2 pos = get_point_pos(0, p_path->get_scale());
	sprite->set_pos(pos);
	// ����ĵ�
	Vector2 faceto = get_point_pos(1, p_path->get_scale());
	float rot = pos.angle_to_point(faceto);
	sprite->set_rot(rot);
	if(hidden)
		sprite->set_opacity(0);

	return true;
}

bool OSpritePath::FishStat::update(const OSpritePath *p_path) {

	if(!Stat::update(p_path))
		return false;

	int index = get_index();
	// �Ѿ�����·��
	if(index >= points.size() / 2) {

		// �������Ƴ���Ļ��
		fish->call("kill", "fadeout");
		return false;
	}
	// ���򲥷���������
	if(!forward)
		index = (points.size() / 2 - 1) - index;

	Vector2 last_pos = sprite->get_pos();
	Vector2 pos = get_point_pos(index, p_path->get_scale());
	// ���ε�λ��һ�£�������
	if(last_pos == pos)
		return true;
	sprite->set_pos(pos);

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
	return true;
}

struct OSpritePath::GroupStat : public OSpritePath::Stat {

	GroupStat() : Stat() {}
	virtual bool init(const OSpritePath *p_path);
	virtual bool update(const OSpritePath *p_path);

	// �������ĵ���������
	int center_index;
	// ���ĵ�����
	Vector2 center_pos;
	// group���ã��Ƿ������ĵ㣩
	bool center;
};

bool OSpritePath::GroupStat::init(const OSpritePath *p_path) {

	// ���ó�ʼλ��/����
	Vector2 pos = get_point_pos(0, p_path->get_scale());
	sprite->set_pos(pos);
	// ����ĵ�
	//	�ж��Ƿ������ĵ�
	Vector2 faceto = center ? (center_pos * p_path->get_scale()) : get_point_pos(1, p_path->get_scale());
	float rot = pos.angle_to_point(faceto);
	sprite->set_rot(rot);
	if(hidden)
		sprite->set_opacity(0);

	return true;
}

bool OSpritePath::GroupStat::update(const OSpritePath *p_path) {

	if(!Stat::update(p_path))
		return false;

	int index = get_index();
	// �Ѿ�����·��
	if(index >= points.size() / 2) {

		// �������Ƴ���Ļ��
		fish->call("kill", "fadeout");
		return false;
	}

	Vector2 last_pos = sprite->get_pos();
	Vector2 pos = get_point_pos(index, p_path->get_scale());
	// ���ε�λ��һ�£�������
	if(last_pos == pos)
		return true;
	sprite->set_pos(pos);

	if(center && (index < center_index)) {

		float rot = pos.angle_to_point(center_pos * p_path->get_scale());
		sprite->set_rot(rot);
	} else {

		// ��������
		float rot = last_pos.angle_to_point(pos);
		float fish_rot = sprite->get_rot();
		float diff_rot = rot - fish_rot;
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

OSpritePath::Stat *OSpritePath::_get_stat(Node *p_fish) const {

	for(const List<Stat *>::Element *E = fishs.front(); E; E = E->next()) {

		Stat *stat = E->get();
		if(stat->fish == p_fish)
			return stat;
	}
	return NULL;
}

bool OSpritePath::set_stat(Node *p_fish, const String& p_key, const Variant& p_value) {

	Stat *stat = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND_V(stat == NULL, false);

	ERR_EXPLAIN("Invalid stat key: " + p_key);
	if(p_key == "speed")
		stat->speed = p_value;
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

real_t OSpritePath::get_length(Node *p_fish) const {

	Stat *stat = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND_V(stat == NULL, false);

	int num_points = stat->points.size() / 2;

	real_t length = num_points / stat->ratio;

	for(const List<Stat::Freeze>::Element *E = stat->freezes.front(); E; E = E->next()) {

		const Stat::Freeze& info = E->get();
		length += (info.end - info.start);
	}

	return length;
}

bool OSpritePath::add_fish(const Dictionary& p_params) {

	Object *fish = p_params["fish"].operator Object *();
	Object *sprite = p_params["sprite"].operator Object *();

	FishStat stat;
	stat.fish = fish ? fish->cast_to<Node>() : NULL;
	stat.sprite = sprite ? sprite->cast_to<OSprite>() : NULL;
	ERR_FAIL_COND_V(stat.fish == NULL || stat.sprite == NULL, false);
	stat.fish_id = stat.fish->get_instance_ID();
	stat.sprite_id = stat.sprite->get_instance_ID();

	stat.hidden = p_params["hidden"];
	stat.delay = p_params["delay"];
	stat.elapsed = p_params["elapsed"];
	stat.speed = p_params["speed"];
	stat.ratio = p_params["ratio"];
	stat.points = p_params["points"];
	stat.forward = p_params["forward"];
	stat.mat = p_params["mat"];

	// tween��ֵ����
	if(p_params.has("tween")) {

		Dictionary d = p_params["tween"];

		FishStat::Tween& tween = stat.tween;
		tween.delta = d["delta"];
		tween.weight = d["weight"];

		Array phases = d["phases"];
		tween.phases.resize(phases.size());

		for(int i = 0; i < phases.size(); i++) {

			Dictionary p = phases[i];

			FishStat::Tween::Phase& phase = tween.phases[i];
			phase.delta = p["delta"];
			phase.weight = p["weight"];
			phase.trans = p["trans"];
			phase.eases = p["eases"];
		}	
	} else {

		// ��tween��Ϣ
		stat.tween.delta = 0;
	}

	if(p_params.has("freezes")) {

		Array freezes = p_params["freezes"];
		for(int i = 0; i < freezes.size(); i++) {

			Dictionary info = freezes[i];
			real_t start = info["start"];
			real_t duration = info["duration"];
			stat.add_freeze_time(start, duration);		
		}	
	}

	fishs.push_back(memnew(FishStat(stat)));

	return stat.init(this);
}

bool OSpritePath::add_group_fish(const Dictionary& p_params) {

	Object *fish = p_params["fish"].operator Object *();
	Object *sprite = p_params["sprite"].operator Object *();

	GroupStat stat;
	stat.fish = fish ? fish->cast_to<Node>() : NULL;
	stat.sprite = sprite ? sprite->cast_to<OSprite>() : NULL;
	ERR_FAIL_COND_V(stat.fish == NULL || stat.sprite == NULL, false);
	stat.fish_id = stat.fish->get_instance_ID();
	stat.sprite_id = stat.sprite->get_instance_ID();

	stat.hidden = p_params["hidden"];
	stat.delay = p_params["delay"];
	stat.elapsed = p_params["elapsed"];
	stat.speed = p_params["speed"];
	stat.ratio = p_params["ratio"];
	stat.points = p_params["points"];
	stat.center_index = p_params["center_index"];
	stat.center_pos = p_params["center_pos"];
	stat.center = p_params["center"];

	if(p_params.has("freezes")) {

		Array freezes = p_params["freezes"];
		for(int i = 0; i < freezes.size(); i++) {

			Dictionary info = freezes[i];
			real_t start = info["start"];
			real_t duration = info["duration"];
			stat.add_freeze_time(start, duration);		
		}	
	}

	fishs.push_back(memnew(GroupStat(stat)));

	return stat.init(this);
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
	return stat->update(this);
}

void OSpritePath::add_freeze_time(Node *p_fish, real_t p_start, real_t p_duration) {

	Stat *stat = _get_stat(p_fish);
	ERR_EXPLAIN("Non-exists fish stat");
	ERR_FAIL_COND(stat == NULL);

	stat->add_freeze_time(p_start, p_duration);
}

void OSpritePath::move(float p_delta) {

	for(List<Stat *>::Element *E = fishs.front(); E;) {

		Stat *stat = E->get();
		stat->elapsed += p_delta * stat->speed;

		if(
			(ObjectDB::get_instance(stat->sprite_id) == NULL) ||
			(ObjectDB::get_instance(stat->fish_id) == NULL) ||
			!stat->update(this)
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

void OSpritePath::clear() {

	for(List<Stat *>::Element *E = fishs.front(); E;) {

		Stat *stat = E->get();
		memdelete(stat);
	}
	fishs.clear();
}

void OSpritePath::set_scale(const Vector2& p_scale) {

	scale = p_scale;
}

Vector2 OSpritePath::get_scale() const {

	return scale;
}

void OSpritePath::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_stat","fish","key","value"),&OSpritePath::set_stat);
	ObjectTypeDB::bind_method(_MD("get_stat","fish","key"),&OSpritePath::get_stat);
	ObjectTypeDB::bind_method(_MD("get_length","fish"),&OSpritePath::get_length);

	ObjectTypeDB::bind_method(_MD("add_fish","params"),&OSpritePath::add_fish);
	ObjectTypeDB::bind_method(_MD("add_group_fish","params"),&OSpritePath::add_group_fish);
	ObjectTypeDB::bind_method(_MD("remove_fish","fish"),&OSpritePath::remove_fish);
	ObjectTypeDB::bind_method(_MD("seek","fish","pos"),&OSpritePath::seek);
	ObjectTypeDB::bind_method(_MD("add_freeze_time","fish","start","duration"),&OSpritePath::add_freeze_time);
	ObjectTypeDB::bind_method(_MD("move","delta"),&OSpritePath::move);
	ObjectTypeDB::bind_method(_MD("clear"),&OSpritePath::clear);
	ObjectTypeDB::bind_method(_MD("set_scale","scale"),&OSpritePath::set_scale);
	ObjectTypeDB::bind_method(_MD("get_scale"),&OSpritePath::get_scale);
}

OSpritePath::OSpritePath()
	: scale(Vector2(1, 1))
{
}

OSpritePath::~OSpritePath() {
}
