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

//#include "core/io/resource_loader.h"
//#include "scene/2d/collision_object_2d.h"
//#include "scene/resources/convex_polygon_shape_2d.h"
//#include "scene/main/resource_preloader.h"
//#include "method_bind_ext.inc"
#include "os/file_access.h"
#include "os/dir_access.h"

#include "modules/digest/text/json_asset.h"

#include "osprite.h"

OSprite::OSpriteResource::OSpriteResource()
	: fps_delta(0)
	, scale(1)
	, shadow_pos(Vector2(0, 0))
	, shadow_scale(1)
	, shown_frame(0)
{
}

OSprite::OSpriteResource::~OSpriteResource() {
}

void OSprite::OSpriteResource::_post_process() {


	for(int i = 0; i < datas.size(); i++) {

		Data& data = datas[i];
		// use previous valid pool data, if current frame.tex is null
		//	for frame texture skip use
		int last_valid_pool = -1;
		for(int j = 0; j < data.pools.size(); j++) {

			Pool& pool = data.pools[j];
			if(pool.frame == -1)
				continue;
			Frame& frame = frames[pool.frame];
			// use previous valid pool data
			if(frame.tex.is_null()) {
				if(last_valid_pool != -1)
					pool = data.pools[last_valid_pool];
				continue;
			}
			last_valid_pool = j;
			// draw anchor pos
			pool.rect.pos += (frame.offset * frame.scale);
			pool.rect.pos *= this->scale;
			// draw rect size
			pool.rect.size = frame.region.size * (this->scale * frame.scale);
			// draw shadow rect
			if(shadow_pos.x != 0 && shadow_pos.y != 0) {
				pool.shadow_rect = pool.rect;
				pool.shadow_rect.pos += shadow_pos;
				pool.shadow_rect.pos *= shadow_scale;
				pool.shadow_rect.size *= shadow_scale;
			}
		}	
	}

	for(int i = 0; i < actions.size(); i++) {

		Action& act = actions[i];
		Data& data = datas[act.index];
		if(act.to == -1)
			act.to = data.pools.size() - 1;
	}
}

void OSprite::OSpriteResource::_parse_blocks(Data& p_data, const Array& p_blocks) {

	p_data.blocks.resize(p_blocks.size());
	for(int i = 0; i < p_blocks.size(); i++) {

		Blocks& blks = p_data.blocks[i];
		Dictionary d = p_blocks[i].operator Dictionary();
		Array boxes = d["fields"].operator Array();
		blks.resize(boxes.size());

		for(int j = 0; j < boxes.size(); j++) {

			Block& blk = blks[j];
			Dictionary d = boxes[j].operator Dictionary();
			// convert box collision to circle(pos+raidus)
			blk.pos.x = d["x"];
			blk.pos.y = d["y"];
			int w = d["width"];
			int h = d["height"];
			blk.radius = (w + h) / 4;

			// adjust by scale
			blk.pos *= scale;
			blk.radius *= scale;
			// plus to circle center pos
			blk.pos += Vector2(blk.radius, blk.radius);
		}
	}	
}

void OSprite::OSpriteResource::_parse_pools(Data& p_data, const Array& p_pools) {

	p_data.pools.resize(p_pools.size());
	for(int i = 0; i < p_pools.size(); i++) {

		Pool& p = p_data.pools[i];
		Dictionary d = p_pools[i].operator Dictionary();

		p.frame = d["frame"];
		p.rect.pos.x = -d["anchor_x"].operator int64_t();
		p.rect.pos.y = -d["anchor_y"].operator int64_t();
	}
}

void OSprite::OSpriteResource::_parse_steps(Data& p_data, const Array& p_steps) {

	p_data.steps.resize(p_steps.size());
	for(int i = 0; i < p_steps.size(); i++) {

		Steps& steps = p_data.steps[i];
		Dictionary d = p_steps[i].operator Dictionary();
		Array boxes = d["fields"].operator Array();
		steps.resize(boxes.size());

		for(int j = 0; j < boxes.size(); j++) {

			Step& step = steps[j];
			Dictionary d = boxes[j].operator Dictionary();

			// TODO: parse step data
		}
	}	
}

static void _parse_rect2(const Dictionary& d, Rect2& rect) {

	rect.pos.x = d["x"];
	rect.pos.y = d["y"];
	rect.size.width = d["w"];
	rect.size.height = d["h"];
}

static void _parse_size(const Dictionary& d, Size2& size) {

	size.width = d["w"];
	size.height = d["h"];
}

bool OSprite::OSpriteResource::_load_texture_pack(const String& p_path, bool p_pixel_alpha) {

	String base_path = p_path.basename();
	String pack_path = base_path + "/";

	Error err;
	DirAccessRef da = DirAccess::open(pack_path, &err);
	if(err != OK)
		return false;

	if(da->list_dir_begin())
		return false;

	bool loaded = false;
	for(;;) {

		String path = da->get_next();
		if(path == "")
			break;
		if(path == "." || path == "..")
			continue;

		String ext = path.extension();
		if(ext != "json")
			continue;

#if defined(IPHONE_ENABLED) || defined(ANDROID_ENABLED)
		String tex_path = pack_path + path.basename() + ".pkm";
#else
		String tex_path = pack_path + path.basename() + ".dds";
#endif

		Ref<Texture> tex = ResourceLoader::load(tex_path, "Texture");
		if(tex.is_null())
			continue;

		path = pack_path + path;
		Ref<JsonAsset> pack = ResourceLoader::load(path, "JsonAsset");
		if(pack.is_null())
			continue;

		Dictionary d = pack->get_value();
		// get texutre pack scale
		Dictionary meta = d["meta"];
		float pack_scale = meta["scale"];
		pack_scale = 1 / pack_scale;

		Dictionary frames = d["frames"];
		Array names = frames.keys();
		for(int i = 0; i < names.size(); i++) {

			String name = names[i];
			String base = name.basename();
			int index = base.to_int();
			if(index >= this->frames.size())
				continue;
			loaded = true;

			Frame& frame = this->frames[index];
			frame.tex = tex;

			Dictionary info = frames[name];

			Rect2 sprite_source_size;
			//Size2 source_size;

			_parse_rect2(info["frame"], frame.region);
			frame.rotated = info["rotated"];
			frame.scale = pack_scale;
			bool trimmed = info["trimmed"];
			if(trimmed) {
				_parse_rect2(info["spriteSourceSize"], sprite_source_size);
				frame.offset = sprite_source_size.pos;
			}
			//if(rotated)
			//	SWAP(frame.region.size.width, frame.region.size.height);
			//_parse_size(info["sourceSize"], source_size);
		}
	}
	da->list_dir_end();
	return loaded;
}

bool OSprite::OSpriteResource::_load_texture_frames(const String& p_path, bool p_pixel_alpha) {

	String base_path = p_path.basename();
	String tex_path = base_path + "/";

	Error err;
	DirAccessRef da = DirAccess::open(tex_path, &err);
	if(err != OK)
		return false;

	if(da->list_dir_begin())
		return false;

	bool loaded = false;
	for(;;) {

		String path = da->get_next();
		if(path == "")
			break;
		if(path == "." || path == "..")
			continue;

		String base = path.basename();
		if(!base.is_numeric())
			continue;

		int index = base.to_int();
		if(index >= frames.size())
			continue;

		path = tex_path + path;
		Frame& frame = frames[index];
		frame.tex = ResourceLoader::load(path, "Texture");
		frame.rotated = false;
		frame.scale = 1;
		if(frame.tex.is_null())
			return false;
		loaded = true;

		frame.region = frame.tex->get_region();
		// etc1(pixel+alpha) texture
		if(p_pixel_alpha)
			frame.region.size.y /= 2;
	}
	da->list_dir_end();
	return loaded;
}

Error OSprite::OSpriteResource::load(const String& p_path) {

	Error err = ERR_CANT_OPEN;
	Ref<JsonAsset> asset = ResourceLoader::load(p_path, "JsonAsset", false, &err);
	ERR_FAIL_COND_V(asset.is_null(), err);
	const Dictionary& d = asset->get_value();
	ERR_FAIL_COND_V(!d.has("action") || !d.has("data") || !d.has("scale") || !d.has("fps"), ERR_FILE_CORRUPT);

	frames.clear();
	actions.clear();
	action_names.clear();
	datas.clear();
	shown_frame = 0;

	bool pixel_alpha = true;
	if(d.has("pixel_alpha"))
		pixel_alpha = d["pixel_alpha"];

	fps_delta = 1.0 / d["fps"].operator double();
	scale = d["scale"];
	if(d.has("shadow_pos")) {

		Array pos = d["shadow_pos"].operator Array();
		shadow_pos = Vector2(pos[0], pos[1]);
		shadow_scale = pos[2];
	}

	Array action = d["action"].operator Array();
	this->actions.resize(action.size());
	for(int i = 0; i < action.size(); i++) {

		Action& act = this->actions[i];

		Dictionary d = action[i].operator Dictionary();
		act.index = d["index"];
		if(d.has("range")) {

			Array range = d["range"].operator Array();
			act.from = range[0];
			act.to = range[1];
		} else {

			act.from = 0;
			act.to = -1;
		}
		if(d.has("block")) {

			Array block = d["block"].operator Array();
			Block blk;
			blk.pos.x = block[0];
			blk.pos.y = block[1];
			blk.radius = block[2];
			act.blocks.push_back(blk);
		}

		act.name = d["name"];
		act.desc = d["desc"];
		if(d.has("pattern"))
			act.pattern = d["pattern"];
		else
			act.pattern = "";
		action_names.set(act.name, &act);
	}

	Array data = d["data"].operator Array();
	this->datas.resize(data.size());
	for(int i = 0; i < data.size(); i++) {

		Data& dat = this->datas[i];

		Dictionary d = data[i].operator Dictionary();
		dat.width = d["width"];
		dat.height = d["height"];
		dat.name = d["name"];
		//d["dummy"];

		if(d.has("blocks"))
			_parse_blocks(dat, d["blocks"]);
		if(d.has("pools"))
			_parse_pools(dat, d["pools"]);
		if(d.has("steps"))
			_parse_steps(dat, d["pools"]);
	}

	Array frames = d["frames"].operator Array();
	this->frames.resize(frames.size());

	if(!_load_texture_pack(p_path, pixel_alpha)) {
		if(!_load_texture_frames(p_path, pixel_alpha)) {
		}
	}

	// calc pool frames rect/src_rect
	_post_process();

	// use previouse frame data, if current frame.tex is null
	//	for texture frame skip use
	int last_valid_frame = -1;
	for(int i = 0; i < frames.size(); i++) {

		Frame& frame = this->frames[i];
		if(frame.tex.is_null()) {
			if(last_valid_frame != -1)
				frame = (Frame&) this->frames[last_valid_frame];
			continue;
		}
		last_valid_frame = i;

		// default shown maximum size texture(if not playing)
		if(this->frames[shown_frame].region.get_size() < frame.region.get_size())
			shown_frame = i;
	}

	return OK;
}

#endif // MODULE_OCEAN_ENABLED
