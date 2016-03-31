/*************************************************************************/
/*  register_types.h                                                     */
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

#include "object_type_db.h"
#include "core/globals.h"
#include "register_types.h"

#include "osprite.h"

#include "core/os/file_access.h"
#include "core/io/resource_loader.h"
#include "scene/resources/texture.h"

typedef Ref<Texture> TextureRef;

class ResourceFormatLoaderOSprite : public ResourceFormatLoader {
public:

	virtual RES load(const String &p_path, const String& p_original_path = "", Error *r_error=NULL) {

		if (r_error)
			*r_error=ERR_CANT_OPEN;

		OSprite::OSpriteResource *res = memnew(OSprite::OSpriteResource);
		Ref<OSprite::OSpriteResource> ref(res);
		ERR_FAIL_COND_V(res->load(p_path) != OK, RES());
		res->set_path(p_path);
		return ref;
	}

	virtual void get_recognized_extensions(List<String> *p_extensions) const {

		p_extensions->push_back("json");
		p_extensions->push_back("schema");
	}

	virtual bool handles_type(const String& p_type) const {

		return p_type=="OSpriteResource";
	}

	virtual String get_resource_type(const String &p_path) const {

		String el = p_path.extension().to_lower();
		if (el=="json" || el=="schema")
			return "OSpriteResource";
		return "";
	}
};

static ResourceFormatLoaderOSprite *resource_loader_osprite = NULL;

void register_ocean_types() {

	ObjectTypeDB::register_type<OSprite>();
	ObjectTypeDB::register_type<OSprite::OSpriteResource>();
	resource_loader_osprite = memnew( ResourceFormatLoaderOSprite );
	ResourceLoader::add_resource_format_loader(resource_loader_osprite);
}

void unregister_ocean_types() {

	if (resource_loader_osprite)
		memdelete(resource_loader_osprite);
}

#else

void register_ocean_types() {}
void unregister_ocean_types() {}

#endif // MODULE_OCEAN_ENABLED
