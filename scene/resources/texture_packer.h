/*************************************************************************/
/*  texture_packer.h                                                     */
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
#ifndef TEXTURE_PACKER_H
#define TEXTURE_PACKER_H

#include "resource.h"
#include "servers/visual_server.h"
#include "math_2d.h"
#include "texture.h"
#include "io/resource_loader.h"

class TexPackAsset : public Resource {

	OBJ_TYPE( TexPackAsset, Resource );
public:
	typedef struct {
		String name;
		Rect2 frame;
		bool rotated, trimmed;
		Rect2 spriteSourceSize;
		Size2 sourceSize;
		Vector2 pivot;
	} Frame;

private:
	float scale;
	Ref<Texture> texture;
	Vector<Frame> frames;

protected:
public:

	Error load(const String& p_path);

	Ref<Texture> get_texture() const;
	const Vector<Frame>& get_frames() const;
	float get_scale() const;

	TexPackAsset();
};

class ResourceFormatLoaderTexPackAsset : public ResourceFormatLoader {
public:

	virtual RES load(const String &p_path, const String& p_original_path = "", Error *r_error=NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String& p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

class TexPackTexture : public Texture {

	OBJ_TYPE( TexPackTexture, Texture );
	RES_BASE_EXTENSION("tptex");
protected:

	Ref<TexPackAsset> asset;
	String atlas_name;
	int atlas_index;

	static void _bind_methods();
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:

	virtual int get_width() const;
	virtual int get_height() const;
	virtual RID get_rid() const;

	virtual bool has_alpha() const;

	virtual void set_flags(uint32_t p_flags);
	virtual uint32_t get_flags() const;

	void set_asset(const Ref<TexPackAsset>& p_asset);
	Ref<TexPackAsset> get_asset() const;

	void set_atlas_name(const String& p_atlas_name);
	const String& get_atlas_name() const;

	virtual void draw(RID p_canvas_item, const Point2& p_pos, const Color& p_modulate=Color(1,1,1), bool p_transpose=false) const;
	virtual void draw_rect(RID p_canvas_item,const Rect2& p_rect, bool p_tile=false,const Color& p_modulate=Color(1,1,1), bool p_transpose=false) const;
	virtual void draw_rect_region(RID p_canvas_item,const Rect2& p_rect, const Rect2& p_src_rect,const Color& p_modulate=Color(1,1,1), bool p_transpose=false) const;
	virtual bool get_rect_region(const Rect2& p_rect, const Rect2& p_src_rect,Rect2& r_rect,Rect2& r_src_rect) const;

	TexPackTexture();
};

#endif
