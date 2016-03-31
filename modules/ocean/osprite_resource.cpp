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

void OSprite::OSpriteResource::_fixup_rects() {

	for(int i = 0; i < pools.size(); i++) {

		OSpriteResource::Pool& pool = pools[i];
		if(pool.frame == -1)
			continue;
		OSpriteResource::Frame& frame = frames[pool.frame];
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

Error OSprite::OSpriteResource::load(const String& p_path) {

	Error err = ERR_CANT_OPEN;
	Ref<JsonAsset> asset = ResourceLoader::load(p_path, "JsonAsset", false, &err);
	ERR_FAIL_COND_V(asset.is_null(), err);
	const Dictionary& d = asset->get_value();
	ERR_FAIL_COND_V(!d.has("action") || !d.has("pool") || !d.has("scale") || !d.has("fps"), ERR_FILE_CORRUPT);

	frames.clear();
	actions.clear();
	action_names.clear();
	blocks.clear();
	pools.clear();
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

		Dictionary a = action[i].operator Dictionary();
		act.name = a["name"];
		act.from = a["from"];
		act.to = a["to"];
		action_names.set(act.name, &act);
	}

	Array block = d["block"].operator Array();
	this->blocks.resize(block.size());
	for(int i = 0; i < block.size(); i++) {

		Blocks& blk = this->blocks[i];

		Dictionary b = block[i].operator Dictionary();
		blk.index = b["index"];
		Array fields = b["fields"].operator Array();
		blk.boxes.resize(fields.size());
		for(int j = 0; j < fields.size(); j++) {

			Rect2& box = blk.boxes[j];
			Dictionary field = fields[j].operator Dictionary();
			box.pos.x = field["x"];
			box.pos.y = field["y"];
			box.size.width = field["width"];
			box.size.height = field["height"];
			// field["reversed"];

			box.pos *= scale;
			box.size *= scale;
		}
	}

	Array pool = d["pool"].operator Array();
	this->pools.resize(pool.size());
	for(int i = 0; i < pool.size(); i++) {

		Pool& p = this->pools[i];

		Dictionary d = pool[i].operator Dictionary();
		p.index = d["index"];	
		p.rect.pos.x = -d["anchor_x"].operator int64_t();
		p.rect.pos.y = -d["anchor_y"].operator int64_t();
		p.frame = d["frame"];
	}

	String tex_path = p_path.substr(0, p_path.rfind(".")) + "/";

	FileAccess *f = FileAccess::create(FileAccess::ACCESS_RESOURCES);

	Array frames = d["frames"].operator Array();
	this->frames.resize(frames.size());
	for(int i = 0; i < frames.size(); i++) {

		Frame& frame = this->frames[i];

		Dictionary d = frames[i].operator Dictionary();
		Array region = d["region"].operator Array();
		String path = tex_path + d["path"].operator String();
		if(f->file_exists(path)) {

			frame.tex = ResourceLoader::load(path, "Texture");
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

	_fixup_rects();

	return OK;
}

#endif // MODULE_OCEAN_ENABLED
