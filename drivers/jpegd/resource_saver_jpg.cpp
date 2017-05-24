/*************************************************************************/
/*  resource_saver_jpg.cpp                                               */
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
#ifdef JPG_ENABLED
#include "resource_saver_jpg.h"
#include "scene/resources/texture.h"
#include "drivers/jpegd/libjpeg/jpeglib.h"
#include "os/file_access.h"
#include "globals.h"
#include "core/image.h"

//static void _write_png_data(png_structp png_ptr,png_bytep data, png_size_t p_length) {
//
//	FileAccess *f = (FileAccess*)png_get_io_ptr(png_ptr);
//	f->store_buffer( (const uint8_t*)data,p_length);
//}

static int JPG_FREAD(void *client_data, unsigned char*ptr, int size) {

	FileAccess* f = (FileAccess *) client_data;
	return f->get_buffer(ptr, size);
}

static int JPG_FWRITE(void *client_data, unsigned char*ptr, int size) {

	FileAccess* f = (FileAccess *) client_data;
	f->store_buffer(ptr, size);
	return size;
}

static int JPG_FFLUSH(void *client_data) {

	//FileAccess* f = (FileAccess *) client_data;
	return 0;
}

static int JPG_FERROR(void *client_data) {

	//FileAccess* f = (FileAccess *) client_data;
	return 0;
}

Error ResourceSaverJPG::save(const String &p_path,const RES& p_resource,uint32_t p_flags) {

	Ref<ImageTexture> texture=p_resource;

	ERR_FAIL_COND_V(!texture.is_valid(),ERR_INVALID_PARAMETER);
	ERR_EXPLAIN("Can't save empty texture as JPG");
	ERR_FAIL_COND_V(!texture->get_width() || !texture->get_height(),ERR_INVALID_PARAMETER);


	Image img = texture->get_data();

	Error err = save_image(p_path, img);

	if (err == OK) {

		bool global_filter = Globals::get_singleton()->get("image_loader/filter");
		bool global_mipmaps = Globals::get_singleton()->get("image_loader/gen_mipmaps");
		bool global_repeat = Globals::get_singleton()->get("image_loader/repeat");

		String text;

		if (global_filter!=bool(texture->get_flags()&Texture::FLAG_FILTER)) {
			text+=bool(texture->get_flags()&Texture::FLAG_FILTER)?"filter=true\n":"filter=false\n";
		}
		if (global_mipmaps!=bool(texture->get_flags()&Texture::FLAG_MIPMAPS)) {
			text+=bool(texture->get_flags()&Texture::FLAG_MIPMAPS)?"gen_mipmaps=true\n":"gen_mipmaps=false\n";
		}
		if (global_repeat!=bool(texture->get_flags()&Texture::FLAG_REPEAT)) {
			text+=bool(texture->get_flags()&Texture::FLAG_REPEAT)?"repeat=true\n":"repeat=false\n";
		}
		if (bool(texture->get_flags()&Texture::FLAG_ANISOTROPIC_FILTER)) {
			text+="anisotropic=true\n";
		}
		if (bool(texture->get_flags()&Texture::FLAG_CONVERT_TO_LINEAR)) {
			text+="tolinear=true\n";
		}
		if (bool(texture->get_flags()&Texture::FLAG_MIRRORED_REPEAT)) {
			text+="mirroredrepeat=true\n";
		}

		if (text!="" || FileAccess::exists(p_path+".flags")) {

			FileAccess* f = FileAccess::open(p_path+".flags",FileAccess::WRITE);
			if (f) {

				f->store_string(text);
				memdelete(f);
			}
		}
	}


	return err;
};

Error ResourceSaverJPG::save_image(const String &p_path, Image &p_img) {

	if (p_img.get_format() > Image::FORMAT_INDEXED_ALPHA)
		p_img.decompress();

	ERR_FAIL_COND_V(p_img.get_format() > Image::FORMAT_INDEXED_ALPHA, ERR_INVALID_PARAMETER);

	Error err;
	FileAccess* f = FileAccess::open(p_path,FileAccess::WRITE,&err);
	if (err) {
		ERR_FAIL_V(err);
	}

    //bool ret = false;
    //do
    //{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
    //    FILE * outfile;                 /* target file */
    JSAMPROW row_pointer[1];        /* pointer to JSAMPLE row[s] */
    int     row_stride;          /* physical row width in image buffer */

	cinfo.client_data = f;
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    //    CC_BREAK_IF((outfile = fopen(FileUtils::getInstance()->getSuitableFOpen(filePath).c_str(), "wb")) == nullptr);

	jpeg_stdio_dest(&cinfo);

	cinfo.image_width = p_img.get_width();    /* image width and height, in pixels */
    cinfo.image_height = p_img.get_height();
    cinfo.input_components = 3;       /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 90, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

	// Convert to rgb format
	p_img.convert(Image::FORMAT_RGB);

    //row_stride = p_img.get_width() * 3; /* JSAMPLEs per row in image_buffer */
	row_stride = p_img.get_width() * p_img.get_format_pixel_size(p_img.get_format());
	DVector<uint8_t>::Read r = p_img.get_data().read();

	while(cinfo.next_scanline < cinfo.image_height) {

		row_pointer[0] = (JSAMPROW) (r.ptr() + cinfo.next_scanline * row_stride);
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	r = DVector<uint8_t>::Read();

    jpeg_finish_compress(&cinfo);
	f->close();
    jpeg_destroy_compress(&cinfo);

	return OK;
}

bool ResourceSaverJPG::recognize(const RES& p_resource) const {

	return (p_resource.is_valid() && p_resource->is_type("ImageTexture"));
}

void ResourceSaverJPG::get_recognized_extensions(const RES& p_resource,List<String> *p_extensions) const{

	if (p_resource->cast_to<Texture>()) {
		p_extensions->push_back("jpg");
		p_extensions->push_back("jpeg");
	}
}

ResourceSaverJPG::ResourceSaverJPG() {

	JFREAD = JPG_FREAD;
	JFWRITE = JPG_FWRITE;
	JFFLUSH = JPG_FFLUSH;
	JFERROR = JPG_FERROR;

	Image::save_jpg_func = &save_image;
};

#endif // JPG_ENABLED
