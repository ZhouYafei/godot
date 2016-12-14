/*************************************************************************/
/*  texture_loader_pvr.cpp                                               */
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
#include "zlib.h"

static void *zlib_alloc_func(void *opaque, uint32_t items, uint32_t size) {
	voidpf ptr =memalloc(items*size);
	zeromem(ptr,items*size);
	return ptr;
}

static void zlib_free_func(void *opaque, void *address) {
	memfree(address);
}

static int zlib_uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) {
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = zlib_alloc_func;
    stream.zfree = zlib_free_func;

    err = inflateInit(&stream);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
            return Z_DATA_ERROR;
        return err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}

static FileAccess *load_pvr_file(const String &p_path, Error& err)
{
    FileAccess *f=FileAccess::open(p_path,FileAccess::READ,&err);
    if(!f)
        return NULL;
    if(p_path.find(".pvr.ccz") != -1)
    {
        struct CCZHeader {
            uint8_t sig[4];             // signature. Should be 'CCZ!' 4 bytes
            uint16_t compression_type;  // should 0
            uint16_t version;           // should be 2 (although version type==1 is also supported)
            uint32_t reserved;          // Reserved for users.
            uint32_t len;               // size of the uncompressed file
        } header;

        enum {
            CCZ_COMPRESSION_ZLIB,       // zlib format.
            CCZ_COMPRESSION_BZIP2,      // bzip2 format (not supported yet)
            CCZ_COMPRESSION_GZIP,       // gzip format (not supported yet)
            CCZ_COMPRESSION_NONE,       // plain (not supported yet)
        };

        if(f->get_len() <= sizeof(header))
        {
            ERR_EXPLAIN("Invalid CCZ file");
            memdelete(f);
            return NULL;
        }
    	f->set_endian_swap(true); //ccz is big endian format
        f->get_buffer((uint8_t *) &header.sig, 4);
        header.compression_type = f->get_16();
        header.version = f->get_16();
        header.reserved = f->get_32();
        header.len = f->get_32();

        // verify header
        if(header.sig[0] != 'C' || header.sig[1] != 'C' || header.sig[2] != 'Z' || header.sig[3] != '!')
        {
            ERR_EXPLAIN("Invalid CCZ file");
            memdelete(f);
            return NULL;
        }
        // verify header version
        if(header.version > 2)
        {
            ERR_EXPLAIN("Unsupported CCZ header format");
            memdelete(f);
            return NULL;
        }
        // verify compression format
        if(header.compression_type != CCZ_COMPRESSION_ZLIB)
        {
            ERR_EXPLAIN("CCZ Unsupported compression method");
            memdelete(f);
            return NULL;
        }
        uint32_t len = header.len;

        Vector<uint8_t> data;
        if(data.resize(len) != OK)
        {
            ERR_EXPLAIN("CCZ: Failed to allocate memory for texture data");
            memdelete(f);
            return NULL;
        }

        uint32_t compressed_size = f->get_len() - f->get_pos();
        Vector<uint8_t> compressed;
        if(compressed.resize(compressed_size) != OK)
        {
            ERR_EXPLAIN("CCZ: Failed to allocate memory for compressed buffer");
            memdelete(f);
            return NULL;
        }
        if(f->get_buffer(&compressed[0], compressed_size) != compressed_size)
        {
            ERR_EXPLAIN("CCZ: Failed to read compressed buffer");
            memdelete(f);
            return NULL;
        }
        memdelete(f);

        uint32_t destlen = len;
        //unsigned long source = (unsigned long) compressed + sizeof(*header);
        int ret = zlib_uncompress(&data[0], (uLongf *) &destlen, &compressed[0], compressed_size);
        if(ret != Z_OK)
        {
            ERR_EXPLAIN("CCZ: Failed to uncompress data");
            memdelete(f);
            return NULL;
        }
        return memnew(FileAccessMemory(data));
    }
    else if(p_path.find(".pvr.gz") != -1)
    {
        // does not support yet
        memdelete(f);
        return NULL;
    }
    else
    {
        return f;
    }
}
