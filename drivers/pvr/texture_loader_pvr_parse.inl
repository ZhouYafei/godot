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
// 'P' 'V' 'R' 3
static const uint32_t PVRTEX3_IDENT = 0x03525650;
static const uint32_t PVRTEX3_IDENT_REV = 0x50565203;

#pragma pack(push, 4)
struct PVRTexHeaderV3
{
	uint32_t version;      /// Version of the file header, used to identify it.
	uint32_t flags;        /// Various format flags.
	uint64_t pixelFormat;  /// The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
	uint32_t colorSpace;   /// The Color Space of the texture, currently either linear RGB or sRGB.
	uint32_t channelType;  /// Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
	uint32_t height;       /// Height of the texture.
	uint32_t width;        /// Width of the texture.
	uint32_t depth;        /// Depth of the texture. (Z-slices)
	uint32_t numSurfaces;  /// Number of members in a Texture Array.
	uint32_t numFaces;     /// Number of faces in a Cube Map. Maybe be a value other than 6.
	uint32_t numMipmaps;   /// Number of MIP Maps in the texture - NB: Includes top level.
	uint32_t metaDataSize; /// Size of the accompanying meta data.
};
#pragma pack(pop)

enum PVRV3PixelFormat
{
	ePVRTPF_PVRTCI_2bpp_RGB = 0,
	ePVRTPF_PVRTCI_2bpp_RGBA,
	ePVRTPF_PVRTCI_4bpp_RGB,
	ePVRTPF_PVRTCI_4bpp_RGBA,
	ePVRTPF_PVRTCII_2bpp = 4,
	ePVRTPF_PVRTCII_4bpp,
	ePVRTPF_ETC1 = 6,
	ePVRTPF_DXT1 = 7,
	ePVRTPF_DXT2,
	ePVRTPF_DXT3,
	ePVRTPF_DXT4,
	ePVRTPF_DXT5,
	ePVRTPF_BC4,
	ePVRTPF_BC5,
	ePVRTPF_BC6,
	ePVRTPF_BC7,
	ePVRTPF_ETC2_RGB = 22,
	ePVRTPF_ETC2_RGBA,
	ePVRTPF_ETC2_RGBA1,
	ePVRTPF_EAC_R = 25,
	ePVRTPF_EAC_RG,
	ePVRTPF_ASTC_4x4 = 27,
	ePVRTPF_ASTC_5x4,
	ePVRTPF_ASTC_5x5,
	ePVRTPF_ASTC_6x5,
	ePVRTPF_ASTC_6x6,
	ePVRTPF_ASTC_8x5,
	ePVRTPF_ASTC_8x6,
	ePVRTPF_ASTC_8x8,
	ePVRTPF_ASTC_10x5,
	ePVRTPF_ASTC_10x6,
	ePVRTPF_ASTC_10x8,
	ePVRTPF_ASTC_10x10,
	ePVRTPF_ASTC_12x10,
	ePVRTPF_ASTC_12x12,
	ePVRTPF_UNKNOWN_FORMAT = 0x7F,

	// v2 format(v3 does not supported)
	ePVRTPF_RGBA4444,
	ePVRTPF_RGBA5551,
	ePVRTPF_RGBA8888,
	ePVRTPF_RGB565,
	ePVRTPF_RGB555,
	ePVRTPF_RGB888,
	ePVRTPF_RGBI8,
	ePVRTPF_RGBAI88,
	ePVRTPF_BGRA8888,
	ePVRTPF_A8,
};

enum PVRV3ChannelType
{
	ePVRTCT_UNORM8 = 0,
	ePVRTCT_SNORM8,
	ePVRTCT_UINT8,
	ePVRTCT_SINT8,
	ePVRTCT_UNORM16,
	ePVRTCT_SNORM16,
	ePVRTCT_UINT16,
	ePVRTCT_SINT16,
	ePVRTCT_UNORM32,
	ePVRTCT_SNORM32,
	ePVRTCT_UINT32,
	ePVRTCT_SINT32,
	ePVRTCT_FLOAT
};

// 'P' 'V' 'R' '!'
static const uint32_t PVRTEX2_IDENT = 0x21525650;
static const uint32_t PVRTEX2_IDENT_REV = 0x50565221;

#pragma pack(push, 4)
struct PVRTexHeaderV2
{
	uint32_t headerSize;
	uint32_t height;
	uint32_t width;
	uint32_t numMipmaps;
	uint32_t flags;
	uint32_t dataSize;
	uint32_t bpp;
	uint32_t bitmaskRed;
	uint32_t bitmaskGreen;
	uint32_t bitmaskBlue;
	uint32_t bitmaskAlpha;
	uint32_t pvrTag;
	uint32_t numSurfaces;
	//ERR_FAIL_COND_V(hsize!=52,RES());
	//uint32_t height = f->get_32();
	//uint32_t width = f->get_32();
	//uint32_t mipmaps = f->get_32();
	//uint32_t flags = f->get_32();
	//uint32_t surfsize = f->get_32();
	//uint32_t bpp = f->get_32();
	//uint32_t rmask = f->get_32();
	//uint32_t gmask = f->get_32();
	//uint32_t bmask = f->get_32();
	//uint32_t amask = f->get_32();
	//uint8_t pvrid[5]={0,0,0,0,0};
	//f->get_buffer(pvrid,4);
	//ERR_FAIL_COND_V(String((char*)pvrid)!="PVR!",RES());
	//uint32_t surfcount = f->get_32();
};
#pragma pack(pop)

// The legacy V2 pixel types we support.
enum PVRPixelTypeV2
{
	PixelTypeRGBA4444 = 0x10,
	PixelTypeRGBA5551 = 0x11,
	PixelTypeRGBA8888 = 0x12,
	PixelTypeRGB565 = 0x13,
	PixelTypeRGB555 = 0x14,
	PixelTypeRGB888 = 0x15,
	PixelTypeRGBI8 = 0x16,
	PixelTypeRGBAI88 = 0x17,
	PixelTypePVRTC2 = 0x18,
	PixelTypePVRTC4,
	PixelTypeBGRA8888 = 0x1A,
	PixelTypeA8 = 0x1B,
	PixelTypePVRTCII2 = 0x1C,
	PixelTypePVRTCII4,
	PixelTypeDXT1 = 0x20,
	PixelTypeDXT3 = 0x22,
	PixelTypeDXT5 = 0x24,
	PixelTypeETC1 = 0x36
};

// The legacy V2 flags
enum PVRFLags2
{
	PVR_HAS_MIPMAPS = 0x00000100,
	PVR_TWIDDLED = 0x00000200,
	PVR_NORMAL_MAP = 0x00000400,
	PVR_BORDER = 0x00000800,
	PVR_CUBE_MAP = 0x00001000,
	PVR_FALSE_MIPMAPS = 0x00002000,
	PVR_VOLUME_TEXTURES = 0x00004000,
	PVR_HAS_ALPHA = 0x00008000,
	PVR_VFLIP = 0x00010000,
};

// Convert a V2 header to V3.
static void _convertPVRHeader(PVRTexHeaderV2 header2, PVRTexHeaderV3 *header3)
{
	// If the header's endianness doesn't match our own, we swap everything.
	if (header2.pvrTag == PVRTEX2_IDENT_REV)
	{
		// All of the struct's members are uint32_t values, so we can do this.
		uint32_t *headerArray = (uint32_t *) &header2;
		for (size_t i = 0; i < sizeof(PVRTexHeaderV2) / sizeof(uint32_t); i++)
			headerArray[i] = BSWAP32(headerArray[i]);
	}

	memset(header3, 0, sizeof(PVRTexHeaderV3));

	header3->version = PVRTEX3_IDENT;
	header3->height = header2.height;
	header3->width = header2.width;
	header3->depth = 1;
	header3->numSurfaces = header2.numSurfaces;
	header3->numFaces = 1;
	header3->numMipmaps = header2.numMipmaps + 1;
	header3->metaDataSize = 0;

	switch ((PVRPixelTypeV2) (header2.flags & 0xFF))
	{
	case PixelTypeRGBA4444:
		header3->pixelFormat = ePVRTPF_RGBA4444;
		break;
	case PixelTypeRGBA5551:
		header3->pixelFormat = ePVRTPF_RGBA5551;
		break;
	case PixelTypeRGBA8888:
		header3->pixelFormat = ePVRTPF_RGBA8888;
		break;
	case PixelTypeRGB565:
		header3->pixelFormat = ePVRTPF_RGB565;
		break;
	case PixelTypeRGB555:
		header3->pixelFormat = ePVRTPF_RGB555;
		break;
	case PixelTypeRGB888:
		header3->pixelFormat = ePVRTPF_RGB888;
		break;
	case PixelTypeRGBI8:
		header3->pixelFormat = ePVRTPF_RGBI8;
		break;
	case PixelTypeRGBAI88:
		header3->pixelFormat = ePVRTPF_RGBAI88;
		break;
	case PixelTypeBGRA8888:
		header3->pixelFormat = ePVRTPF_BGRA8888;
		break;
	case PixelTypeA8:
		header3->pixelFormat = ePVRTPF_A8;
		break;
	case PixelTypePVRTC2:
		header3->pixelFormat = (header2.flags & PVR_HAS_ALPHA) ? ePVRTPF_PVRTCI_2bpp_RGBA : ePVRTPF_PVRTCI_2bpp_RGB;
		break;
	case PixelTypePVRTC4:
		header3->pixelFormat = (header2.flags & PVR_HAS_ALPHA) ? ePVRTPF_PVRTCI_4bpp_RGBA : ePVRTPF_PVRTCI_4bpp_RGB;
		break;
	case PixelTypePVRTCII2:
		header3->pixelFormat = ePVRTPF_PVRTCII_2bpp;
		break;
	case PixelTypePVRTCII4:
		header3->pixelFormat = ePVRTPF_PVRTCII_4bpp;
		break;
	case PixelTypeDXT1:
		header3->pixelFormat = ePVRTPF_DXT1;
		break;
	case PixelTypeDXT3:
		header3->pixelFormat = ePVRTPF_DXT3;
		break;
	case PixelTypeDXT5:
		header3->pixelFormat = ePVRTPF_DXT5;
		break;
	case PixelTypeETC1:
		header3->pixelFormat = ePVRTPF_ETC1;
		break;
	default:
		header3->pixelFormat = ePVRTPF_UNKNOWN_FORMAT;
		break;
	}
}

static Image::Format _convertFormat(PVRV3PixelFormat format, PVRV3ChannelType channeltype)
{
	bool snorm = false;

	switch (channeltype)
	{
	case ePVRTCT_SNORM8:
	case ePVRTCT_SNORM16:
	case ePVRTCT_SNORM32:
		snorm = true;
		break;
	default:
		break;
	}

	switch (format)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
		return Image::FORMAT_PVRTC2;
	case ePVRTPF_PVRTCI_2bpp_RGBA:
		return Image::FORMAT_PVRTC2_ALPHA;
	case ePVRTPF_PVRTCI_4bpp_RGB:
		return Image::FORMAT_PVRTC4;
	case ePVRTPF_PVRTCI_4bpp_RGBA:
		return Image::FORMAT_PVRTC4_ALPHA;
	case ePVRTPF_ETC1:
		return Image::FORMAT_ETC;
	case ePVRTPF_DXT1:
		return Image::FORMAT_BC1;
	case ePVRTPF_DXT3:
		return Image::FORMAT_BC3;
	case ePVRTPF_DXT5:
		return Image::FORMAT_BC5;
	//case ePVRTPF_BC4:
	//	return snorm ? Image::FORMAT_BC4s : Image::FORMAT_BC4;
	//case ePVRTPF_BC5:
	//	return snorm ? Image::FORMAT_BC5s : Image::FORMAT_BC5;
	//case ePVRTPF_BC6:
	//	return snorm ? Image::FORMAT_BC6Hs : Image::FORMAT_BC6H;
	//case ePVRTPF_BC7:
	//	return Image::FORMAT_BC7;
	//case ePVRTPF_ETC2_RGB:
	//	return Image::FORMAT_ETC2_RGB;
	//case ePVRTPF_ETC2_RGBA:
	//	return Image::FORMAT_ETC2_RGBA;
	//case ePVRTPF_ETC2_RGBA1:
	//	return Image::FORMAT_ETC2_RGBA1;
	//case ePVRTPF_EAC_R:
	//	return snorm ? Image::FORMAT_EAC_Rs : Image::FORMAT_EAC_R;
	//case ePVRTPF_EAC_RG:
	//	return snorm ? Image::FORMAT_EAC_RGs : Image::FORMAT_EAC_RG;
	//case ePVRTPF_ASTC_4x4:
	//	return Image::FORMAT_ASTC_4x4;
	//case ePVRTPF_ASTC_5x4:
	//	return Image::FORMAT_ASTC_5x4;
	//case ePVRTPF_ASTC_5x5:
	//	return Image::FORMAT_ASTC_5x5;
	//case ePVRTPF_ASTC_6x5:
	//	return Image::FORMAT_ASTC_6x5;
	//case ePVRTPF_ASTC_6x6:
	//	return Image::FORMAT_ASTC_6x6;
	//case ePVRTPF_ASTC_8x5:
	//	return Image::FORMAT_ASTC_8x5;
	//case ePVRTPF_ASTC_8x6:
	//	return Image::FORMAT_ASTC_8x6;
	//case ePVRTPF_ASTC_8x8:
	//	return Image::FORMAT_ASTC_8x8;
	//case ePVRTPF_ASTC_10x5:
	//	return Image::FORMAT_ASTC_10x5;
	//case ePVRTPF_ASTC_10x6:
	//	return Image::FORMAT_ASTC_10x6;
	//case ePVRTPF_ASTC_10x8:
	//	return Image::FORMAT_ASTC_10x8;
	//case ePVRTPF_ASTC_10x10:
	//	return Image::FORMAT_ASTC_10x10;
	//case ePVRTPF_ASTC_12x10:
	//	return Image::FORMAT_ASTC_12x10;
	//case ePVRTPF_ASTC_12x12:
	//	return Image::FORMAT_ASTC_12x12;
	case ePVRTPF_RGBA4444:
		return Image::FORMAT_RGBA_4444;
	case ePVRTPF_RGBA5551:
		return Image::FORMAT_RGBA_5551;
	case ePVRTPF_RGBA8888:
		return Image::FORMAT_RGBA;
	case ePVRTPF_RGB565:
		return Image::FORMAT_RGB_565;
	case ePVRTPF_RGB555:
		return Image::FORMAT_RGBA_5551;
	case ePVRTPF_RGB888:
		return Image::FORMAT_RGB;
	case ePVRTPF_RGBI8:
		return Image::FORMAT_INDEXED;
	case ePVRTPF_RGBAI88:
		return Image::FORMAT_INDEXED_ALPHA;
	case ePVRTPF_BGRA8888:
		return Image::FORMAT_BGRA_8888;
	case ePVRTPF_A8:
		return Image::FORMAT_ALPHA_8;
	default:
		return Image::FORMAT_CUSTOM;
	}
}

static int _getBitsPerPixel(uint64_t pixelformat)
{
	// Uncompressed formats have their bits per pixel stored in the high bits.
	if ((pixelformat & 0xFFFFFFFF) != pixelformat)
	{
		const uint8_t *charformat = (const uint8_t *) &pixelformat;
		return charformat[4] + charformat[5] + charformat[6] + charformat[7];
	}

	switch (pixelformat)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
	case ePVRTPF_PVRTCI_2bpp_RGBA:
	case ePVRTPF_PVRTCII_2bpp:
		return 2;
	case ePVRTPF_PVRTCI_4bpp_RGB:
	case ePVRTPF_PVRTCI_4bpp_RGBA:
	case ePVRTPF_PVRTCII_4bpp:
	case ePVRTPF_ETC1:
	case ePVRTPF_DXT1:
	case ePVRTPF_BC4:
	case ePVRTPF_ETC2_RGB:
	case ePVRTPF_ETC2_RGBA1:
	case ePVRTPF_EAC_R:
		return 4;
	case ePVRTPF_DXT2:
	case ePVRTPF_DXT3:
	case ePVRTPF_DXT4:
	case ePVRTPF_DXT5:
	case ePVRTPF_BC5:
	case ePVRTPF_BC6:
	case ePVRTPF_BC7:
	case ePVRTPF_ETC2_RGBA:
	case ePVRTPF_EAC_RG:
		return 8;

	case ePVRTPF_RGBA4444:
		return 16;
	case ePVRTPF_RGBA5551:
		return 16;
	case ePVRTPF_RGBA8888:
		return 32;
	case ePVRTPF_RGB565:
		return 16;
	case ePVRTPF_RGB555:
		return 16;
	case ePVRTPF_RGB888:
		return 24;
	case ePVRTPF_RGBI8:
		return 8;
	case ePVRTPF_RGBAI88:
		return 16;
	case ePVRTPF_BGRA8888:
		return 32;
	case ePVRTPF_A8:
		return 8;

	default:
		return 0;
	}
}

static void _getFormatMinDimensions(uint64_t pixelformat, int &minX, int &minY, int &minZ)
{
	minZ = 1;

	switch (pixelformat)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
	case ePVRTPF_PVRTCI_2bpp_RGBA:
		minX = 16;
		minY = 8;
		break;
	case ePVRTPF_PVRTCI_4bpp_RGB:
	case ePVRTPF_PVRTCI_4bpp_RGBA:
		minX = minY = 8;
		break;
	case ePVRTPF_PVRTCII_2bpp:
		minX = 8;
		minY = 4;
		break;
	case ePVRTPF_PVRTCII_4bpp:
		minX = minY = 4;
		break;
	case ePVRTPF_DXT1:
	case ePVRTPF_DXT2:
	case ePVRTPF_DXT3:
	case ePVRTPF_DXT4:
	case ePVRTPF_DXT5:
	case ePVRTPF_BC4:
	case ePVRTPF_BC5:
	case ePVRTPF_BC6:
	case ePVRTPF_BC7:
	case ePVRTPF_ETC1:
	case ePVRTPF_ETC2_RGB:
	case ePVRTPF_ETC2_RGBA:
	case ePVRTPF_ETC2_RGBA1:
	case ePVRTPF_EAC_R:
	case ePVRTPF_EAC_RG:
		minX = minY = 4;
		break;
	case ePVRTPF_ASTC_4x4:
		minX = 4;
		minY = 4;
		break;
	case ePVRTPF_ASTC_5x4:
		minX = 5;
		minY = 4;
		break;
	case ePVRTPF_ASTC_5x5:
		minX = 5;
		minY = 5;
		break;
	case ePVRTPF_ASTC_6x5:
		minX = 6;
		minY = 5;
		break;
	case ePVRTPF_ASTC_6x6:
		minX = 6;
		minY = 6;
		break;
	case ePVRTPF_ASTC_8x5:
		minX = 8;
		minY = 5;
		break;
	case ePVRTPF_ASTC_8x6:
		minX = 8;
		minY = 6;
		break;
	case ePVRTPF_ASTC_8x8:
		minX = 8;
		minY = 8;
		break;
	case ePVRTPF_ASTC_10x5:
		minX = 10;
		minY = 5;
		break;
	case ePVRTPF_ASTC_10x6:
		minX = 10;
		minY = 6;
		break;
	case ePVRTPF_ASTC_10x8:
		minX = 10;
		minY = 8;
		break;
	case ePVRTPF_ASTC_10x10:
		minX = 10;
		minY = 10;
		break;
	case ePVRTPF_ASTC_12x10:
		minX = 12;
		minY = 10;
		break;
	case ePVRTPF_ASTC_12x12:
		minX = 12;
		minY = 12;
		break;
	default: // We don't handle all possible formats, but that's fine.
		minX = minY = 1;
		break;
	}
}

static size_t _getMipLevelSize(const PVRTexHeaderV3 &header, int miplevel)
{
	int smallestwidth = 1;
	int smallestheight = 1;
	int smallestdepth = 1;
	_getFormatMinDimensions(header.pixelFormat, smallestwidth, smallestheight, smallestdepth);

	int width = MAX((int) header.width >> miplevel, 1);
	int height = MAX((int) header.height >> miplevel, 1);
	int depth = MAX((int) header.depth >> miplevel, 1);

	// Pad the dimensions.
	width = ((width + smallestwidth - 1) / smallestwidth) * smallestwidth;
	height = ((height + smallestheight - 1) / smallestheight) * smallestheight;
	depth = ((depth + smallestdepth - 1) / smallestdepth) * smallestdepth;

	if (header.pixelFormat >= ePVRTPF_ASTC_4x4 && header.pixelFormat <= ePVRTPF_ASTC_12x12)
		return (width / smallestwidth) * (height / smallestheight) * (depth / smallestdepth) * (128 / 8);
	else
		return _getBitsPerPixel(header.pixelFormat) * width * height * depth / 8;
}

//	Image::Format format=Image::FORMAT_MAX;
//
//
//	switch(flags&0xFF) {
//
//        case kPVR2TexturePixelFormat_RGBA_4444:
//            format=Image::FORMAT_RGBA_4444;
//            break;
//        case kPVR2TexturePixelFormat_RGBA_5551:
//            format=Image::FORMAT_RGBA_5551;
//            break;
//        case kPVR2TexturePixelFormat_RGB_565:
//            format=Image::FORMAT_RGB_565;
//            break;
//        //case kPVR2TexturePixelFormat_RGB_555:
//        //    format=Image::FORMAT_RGB_555;
//        //    break;
//        //case kPVR2TexturePixelFormat_BGRA_8888:
//        //    format=Image::FORMAT_BGRA_8888;
//        //    break;
//        case kPVR2TexturePixelFormat_A_8:
//            format=Image::FORMAT_ALPHA_8;
//            break;
//		case kPVR2TexturePixelFormat_PVRTC_2BPP_RGBA:
//		case 0xC:
//            format=(flags&PVR_HAS_ALPHA)?Image::FORMAT_PVRTC2_ALPHA:Image::FORMAT_PVRTC2;
//            break;
//
//		case kPVR2TexturePixelFormat_PVRTC_4BPP_RGBA:
//		case 0xD:
//            format=(flags&PVR_HAS_ALPHA)?Image::FORMAT_PVRTC4_ALPHA:Image::FORMAT_PVRTC4;
//            break;
//
//		case kPVR2TexturePixelFormat_I_8:
//			format=Image::FORMAT_GRAYSCALE;
//            break;
//
//		case kPVR2TexturePixelFormat_AI_88:
//			format=Image::FORMAT_GRAYSCALE_ALPHA;
//            break;
//
//		case 0x20:
//		case 0x80:
//		case 0x81:
//			format=Image::FORMAT_BC1;
//            break;
//
//		case 0x21:
//		case 0x22:
//		case 0x82:
//		case 0x83:
//			format=Image::FORMAT_BC2;
//            break;
//
//		case 0x23:
//		case 0x24:
//		case 0x84:
//		case 0x85:
//			format=Image::FORMAT_BC3;
//            break;
//
//		case 0x4:
//		case kPVR2TexturePixelFormat_RGB_888:
//			format=Image::FORMAT_RGB;
//            break;
//
//		case 0x5:
//		case kPVR2TexturePixelFormat_RGBA_8888:
//			format=Image::FORMAT_RGBA;
//            break;
//
//		case 0x36:
//			format=Image::FORMAT_ETC;
//            break;
//
//		default:
//			ERR_EXPLAIN("Unsupported format in PVR texture: "+itos(flags&0xFF));
//			ERR_FAIL_V(RES());
//
//	}
//
//	w = DVector<uint8_t>::Write();
//
//	int tex_flags=Texture::FLAG_FILTER|Texture::FLAG_REPEAT;
//
//	if (mipmaps)
//		tex_flags|=Texture::FLAG_MIPMAPS;
//
//
//	print_line("flip: "+itos(flags&PVR_VFLIP));
//
//	Image image(width,height,mipmaps,format,data);
//	ERR_FAIL_COND_V(image.empty(),RES());
