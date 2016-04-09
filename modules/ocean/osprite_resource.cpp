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
		for(int j = 0; j < data.pools.size(); j++) {

			Pool& pool = data.pools[j];
			if(pool.frame == -1)
				continue;
			Frame& frame = frames[pool.frame];
			// draw anchor pos
			pool.rect.pos *= scale;
			// draw rect size
			pool.rect.size = frame.region.size * scale;
			// draw shadow rect
			pool.shadow_rect = pool.rect;
			pool.shadow_rect.pos += shadow_pos;
			pool.shadow_rect.pos *= shadow_scale;
			pool.shadow_rect.size *= shadow_scale;
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

	String tex_path = p_path.substr(0, p_path.rfind(".")) + "/";

	FileAccess *f = FileAccess::create(FileAccess::ACCESS_RESOURCES);

	Array frames = d["frames"].operator Array();
	this->frames.resize(frames.size());
	int last_valid_texi = -1;
	for(int i = 0; i < frames.size(); i++) {

		Frame& frame = this->frames[i];

		Dictionary d = frames[i].operator Dictionary();
		Array region = d["region"].operator Array();
		String path = tex_path + d["path"].operator String();
		if(f->file_exists(path)) {

			frame.tex = ResourceLoader::load(path, "Texture");
			// use last valid frame info(tex/region) if current frame texture is not exists
			if(!frame.tex.is_valid() && last_valid_texi != -1) {
				frame = (Frame&) frames[last_valid_texi];
				continue;
			}
			last_valid_texi = i;

			if(region.size() == 4)
				frame.region = Rect2(region[0], region[1], region[2], region[3]);
			else if(frame.tex.is_valid()) {

				frame.region = frame.tex->get_region();
				// etc1(pixel+alpha) texture
				if(pixel_alpha)
					frame.region.size.y /= 2;

				// default shown maximum size texture(if not playing)
				if(this->frames[shown_frame].tex->get_size() < frame.tex->get_size())
					shown_frame = i;
			} else {

				frame.region = Rect2(0, 0, 0, 0);
			}			
		} else {

			frame.tex = RES();
		}
	}
	memdelete(f);

	_post_process();

	return OK;
}

#endif // MODULE_OCEAN_ENABLED
