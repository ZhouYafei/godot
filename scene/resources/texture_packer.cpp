/*************************************************************************/
/*  texture_packer.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2016 Juan Linietsky, Ariel Manzur.                 */
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
#include "texture_packer.h"
#include "modules/digest/text/json_asset.h"

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

static void _parse_vector2(const Dictionary& d, Vector2& vec) {

	vec.x = d["x"];
	vec.y = d["y"];
}

Error TexPackAsset::load(const String& p_path) {

	Ref<JsonAsset> pack = ResourceLoader::load(p_path, "JsonAsset");
	if(pack.is_null())
		return ERR_CANT_OPEN;

	Dictionary d = pack->get_value();
	if(!d.has("meta"))
		return ERR_INVALID_DATA;
	// get texutre pack scale
	Dictionary meta = d["meta"];
	this->scale = 1 / meta["scale"].operator real_t();

	String base_path = p_path.substr(0, p_path.find_last("/") + 1);
	String path = meta["image"];
#if defined(IPHONE_ENABLED) || defined(ANDROID_ENABLED)
	String tex_path = base_path + path.basename() + ".pkm";
#else
	String tex_path = base_path + path.basename() + ".dds";
#endif
	this->texture = ResourceLoader::load(tex_path, "Texture");
	if(this->texture.is_null())
		return ERR_CANT_OPEN;

	int index = 0;
	Dictionary frames = d["frames"];
	this->frames.resize(frames.size());
	Array names = frames.keys();
	for(int i = 0; i < names.size(); i++) {

		Frame& frame = this->frames[index++];
		frame.name = names[i];

		Dictionary info = frames[frame.name];
		_parse_rect2(info["frame"], frame.frame);
		frame.rotated = info["rotated"];
		frame.trimmed = info["trimmed"];
		_parse_rect2(info["spriteSourceSize"], frame.spriteSourceSize);
		_parse_size(info["sourceSize"], frame.sourceSize);
		_parse_vector2(info["pivot"], frame.pivot);
	}
	return OK;
}

Ref<Texture> TexPackAsset::get_texture() const {

	return texture;
}

const Vector<TexPackAsset::Frame>& TexPackAsset::get_frames() const {

	return frames;
}

float TexPackAsset::get_scale() const {

	return scale;
}

TexPackAsset::TexPackAsset()
	: scale(1)
{
}

//////////////////////////////////////////

RES ResourceFormatLoaderTexPackAsset::load(const String &p_path, const String& p_original_path, Error *r_error) {

	if (r_error)
		*r_error=ERR_CANT_OPEN;

	RES res;
	Error err;

	if(p_path.ends_with(".json")) {

		TexPackAsset *asset = memnew(TexPackAsset);
		err = asset->load(p_path);
		if(err != OK)
			return RES();
		res = Ref<TexPackAsset>(asset);
	}
	if(r_error != NULL)
		*r_error = err;
	return res;
}

void ResourceFormatLoaderTexPackAsset::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("json");
}

bool ResourceFormatLoaderTexPackAsset::handles_type(const String& p_type) const {

	return p_type=="TexPackAsset";
}

String ResourceFormatLoaderTexPackAsset::get_resource_type(const String &p_path) const {

	String el = p_path.extension().to_lower();
	if (el != "json")
		return "";

	Ref<JsonAsset> pack = ResourceLoader::load(p_path, "JsonAsset");
	if(pack.is_null())
		return "";

	if(pack->get_value().get_type() != Variant::DICTIONARY)
		return "";

	Dictionary d = pack->get_value();
	if(!d.has("meta") || d["meta"].get_type() != Variant::DICTIONARY)
		return "";

	Dictionary meta = d["meta"];
	if(!d.has("app") || !d.has("scale"))
		return "";

	if(d["app"] != String("http://www.codeandweb.com/texturepacker"))
		return "";

	return "TexPackAsset";
}

//////////////////////////////////////////

int TexPackTexture::get_width() const {

	if(atlas_index == -1)
		return 0;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	return frame.sourceSize.width * asset->get_scale();
}

int TexPackTexture::get_height() const {

	if(atlas_index == -1)
		return 0;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	return frame.sourceSize.height * asset->get_scale();
}

Rect2 TexPackTexture::get_region() const {

	if(atlas_index == -1)
		return Rect2();

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	return frame.frame;
}

bool TexPackTexture::is_rotated() const {

	if(atlas_index == -1)
		return false;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	return frame.rotated;
}

RID TexPackTexture::get_rid() const {

	if(asset.is_valid() && asset->get_texture().is_valid())
		return asset->get_texture()->get_rid();

	return RID();
}

bool TexPackTexture::has_alpha() const {

	if(asset.is_valid() && asset->get_texture().is_valid())
		return asset->get_texture()->has_alpha();

	return false;
}

void TexPackTexture::set_flags(uint32_t p_flags) {

	if(asset.is_valid() && asset->get_texture().is_valid())
		asset->get_texture()->set_flags(p_flags);
}

uint32_t TexPackTexture::get_flags() const{

	if(asset.is_valid() && asset->get_texture().is_valid())
		return asset->get_texture()->get_flags();

	return 0;
}

void TexPackTexture::set_asset(const Ref<TexPackAsset>& p_asset) {

	asset = p_asset;
	atlas_index = -1;
	atlas_name = "[NONE]";

	emit_changed();
	_change_notify();
}

Ref<TexPackAsset> TexPackTexture::get_asset() const {

	return asset;
}

void TexPackTexture::set_atlas_name(const String& p_atlas_name) {

	atlas_name = p_atlas_name;

	if(!asset.is_valid())
		return;

	atlas_index = -1;
	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	for(size_t i = 0; i < frames.size(); i++) {

		const TexPackAsset::Frame& frame = frames[i];
		if(frame.name == p_atlas_name) {

			atlas_index = i;
			break;
		}
	}
	emit_changed();
	_change_notify();
}

const String& TexPackTexture::get_atlas_name() const {

	return atlas_name;
}

void TexPackTexture::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_asset","asset:TexPackAsset"),&TexPackTexture::set_asset);
	ObjectTypeDB::bind_method(_MD("get_asset:TexPackAsset"),&TexPackTexture::get_asset);

	ObjectTypeDB::bind_method(_MD("set_atlas_name","atlas_name"),&TexPackTexture::set_atlas_name);
	ObjectTypeDB::bind_method(_MD("get_atlas_name"),&TexPackTexture::get_atlas_name);

	ObjectTypeDB::bind_method(_MD("get_region"),&TexPackTexture::get_region);
	ObjectTypeDB::bind_method(_MD("is_rotated"),&TexPackTexture::is_rotated);

	ADD_PROPERTY( PropertyInfo( Variant::OBJECT, "asset", PROPERTY_HINT_RESOURCE_TYPE,"TexPackAsset"), _SCS("set_asset"),_SCS("get_asset") );
}

bool TexPackTexture::_set(const StringName& p_name, const Variant& p_value) {

	if(p_name == "atlas_name")
		set_atlas_name(p_value);
	else
		return false;

	return true;
}

bool TexPackTexture::_get(const StringName& p_name, Variant &r_ret) const {

	if(p_name == "atlas_name")
		r_ret = get_atlas_name();
	else
		return false;

	return true;
}

void TexPackTexture::_get_property_list(List<PropertyInfo> *p_list) const {

	String hint = "[NONE]";
	if(asset.is_valid()) {

		Vector<String> names;
		const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
		for(size_t i = 0; i < frames.size(); i++) {

			const TexPackAsset::Frame& frame = frames[i];
			names.push_back(frame.name);
		}
		names.sort();

		for(size_t i = 0; i < names.size(); i++)
			hint += ("," + names[i]);
	}
	p_list->push_back(PropertyInfo( Variant::STRING, "atlas_name", PROPERTY_HINT_ENUM, hint));
}

void TexPackTexture::draw(RID p_canvas_item, const Point2& p_pos, const Color& p_modulate, bool p_transpose) const {

	if(atlas_index == -1)
		return;

	Ref<Texture> atlas = asset->get_texture();
	if (!atlas.is_valid())
		return;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	float scale = asset->get_scale();

	const Rect2& srect = frame.spriteSourceSize;
	Rect2 rect = Rect2(p_pos, get_size());
	rect.pos += rect.size * (srect.pos / frame.sourceSize);
	rect.size -= rect.size * ((frame.sourceSize - (srect.pos + srect.size)) / frame.sourceSize);

	Rect2 src_rect = frame.frame;
	if(frame.rotated) {

		SWAP(rect.size.x, rect.size.y);
		rect.size.y = -rect.size.y;
		SWAP(src_rect.size.x, src_rect.size.y);
		p_transpose = !p_transpose;
	}

	VS::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, rect, atlas->get_rid(), src_rect, p_modulate, p_transpose);
}

void TexPackTexture::draw_rect(RID p_canvas_item,const Rect2& p_rect, bool p_tile,const Color& p_modulate, bool p_transpose) const {

	if(atlas_index == -1)
		return;

	Ref<Texture> atlas = asset->get_texture();
	if (!atlas.is_valid())
		return;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];
	float scale = asset->get_scale();

	const Rect2& srect = frame.spriteSourceSize;
	Rect2 rect = p_rect;
	rect.pos += rect.size * (srect.pos / frame.sourceSize);
	rect.size -= rect.size * ((frame.sourceSize - (srect.pos + srect.size)) / frame.sourceSize);

	Rect2 src_rect = frame.frame;
	if(frame.rotated) {

		SWAP(rect.size.x, rect.size.y);
		rect.size.y = -rect.size.y;
		SWAP(src_rect.size.x, src_rect.size.y);
		p_transpose = !p_transpose;
	}

	VS::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, rect, atlas->get_rid(), src_rect, p_modulate, p_transpose);
}

void TexPackTexture::draw_rect_region(RID p_canvas_item,const Rect2& p_rect, const Rect2& p_src_rect,const Color& p_modulate, bool p_transpose) const {

	if(atlas_index == -1)
		return;

	Ref<Texture> atlas = asset->get_texture();
	if (!atlas.is_valid())
		return;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];

	const Rect2& srect = frame.spriteSourceSize;
	Rect2 rect = p_rect;
	rect.pos += rect.size * (srect.pos / frame.sourceSize);
	rect.size -= rect.size * ((frame.sourceSize - (srect.pos + srect.size)) / frame.sourceSize);

	float scale = asset->get_scale();
	Rect2 src_rect = p_src_rect;
	src_rect.pos /= scale;
	src_rect.pos += frame.frame.pos;
	src_rect.size /= scale;

	if(frame.rotated) {

		SWAP(rect.size.x, rect.size.y);
		rect.size.y = -rect.size.y;
		SWAP(src_rect.size.x, src_rect.size.y);
		p_transpose = !p_transpose;
	}
	VS::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, rect, atlas->get_rid(), src_rect, p_modulate, p_transpose);
}

bool TexPackTexture::get_rect_region(const Rect2& p_rect, const Rect2& p_src_rect,Rect2& r_rect,Rect2& r_src_rect) const {

	if(atlas_index == -1)
		return false;

	Ref<Texture> atlas = asset->get_texture();
	if (!atlas.is_valid())
		return false;

	const Vector<TexPackAsset::Frame>& frames = asset->get_frames();
	const TexPackAsset::Frame& frame = frames[atlas_index];

	const Rect2& srect = frame.spriteSourceSize;
	Rect2 rect = p_rect;
	rect.pos += rect.size * (srect.pos / frame.sourceSize);
	rect.size -= rect.size * ((frame.sourceSize - (srect.pos + srect.size)) / frame.sourceSize);

	float scale = asset->get_scale();
	Rect2 src_rect = p_src_rect;
	src_rect.pos /= scale;
	src_rect.pos += frame.frame.pos;
	src_rect.size /= scale;

	r_rect = rect;
	r_src_rect = src_rect;

	return true;
}


TexPackTexture::TexPackTexture()
	: asset(RES())
	, atlas_name("[NONE]")
	, atlas_index(-1)
{
}
