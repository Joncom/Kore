#include "pch.h"

#include "TextureImpl.h"
#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Log.h>

using namespace Kore;

#ifndef GL_TEXTURE_3D
#define GL_TEXTURE_3D 0x806F
#endif

namespace {
	int convertFormat(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA32:
		case Graphics4::Image::RGBA64:
		case Graphics4::Image::RGBA128:
		default:
			// #ifdef GL_BGRA
			// return GL_BGRA;
			// #else
			return GL_RGBA;
		// #endif
		case Graphics4::Image::RGB24:
			return GL_RGB;
		case Graphics4::Image::A32:
		case Graphics4::Image::Grey8:
#ifdef OPENGLES
			return GL_LUMINANCE;
#else
			return GL_RED;
#endif
		}
	}

	int convertInternalFormat(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA128:
#ifdef GL_ARB_texture_float
			return GL_RGBA32F;
#else
			return GL_RGBA;
#endif
		case Graphics4::Image::RGBA32:
		case Graphics4::Image::RGBA64:
		default:
			// #ifdef GL_BGRA
			// return GL_BGRA;
			// #else
			return GL_RGBA;
		// #endif
		case Graphics4::Image::RGB24:
			return GL_RGB;
		case Graphics4::Image::A32:
#ifdef OPENGLES
			return GL_LUMINANCE;
#else
			return GL_R8;
#endif
		case Graphics4::Image::Grey8:
#ifdef OPENGLES
			return GL_LUMINANCE;
#else
			return GL_RED;
#endif
		}
	}

	int convertType(Graphics4::Image::Format format) {
		switch (format) {
		case Graphics4::Image::RGBA128:
		case Graphics4::Image::RGBA64:
		case Graphics4::Image::A32:
			return GL_FLOAT;
		case Graphics4::Image::RGBA32:
		default:
			return GL_UNSIGNED_BYTE;
		}
	}

#if 0
	int astcFormat(u8 blockX, u8 blockY) {
		switch (blockX) {
		case 4:
			switch (blockY) {
			case 4:
				return COMPRESSED_RGBA_ASTC_4x4_KHR;
			}
		case 5:
			switch (blockY) {
			case 4:
				return COMPRESSED_RGBA_ASTC_5x4_KHR;
			case 5:
				return COMPRESSED_RGBA_ASTC_5x5_KHR;
			}
		case 6:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_6x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_6x6_KHR;
			}
		case 8:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_8x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_8x6_KHR;
			case 8:
				return COMPRESSED_RGBA_ASTC_8x8_KHR;
			}
		case 10:
			switch (blockY) {
			case 5:
				return COMPRESSED_RGBA_ASTC_10x5_KHR;
			case 6:
				return COMPRESSED_RGBA_ASTC_10x6_KHR;
			case 8:
				return COMPRESSED_RGBA_ASTC_10x8_KHR;
			case 10:
				return COMPRESSED_RGBA_ASTC_10x10_KHR;
			}
		case 12:
			switch (blockY) {
			case 10:
				return COMPRESSED_RGBA_ASTC_12x10_KHR;
			case 12:
				return COMPRESSED_RGBA_ASTC_12x12_KHR;
			}
		}
		return 0;
	}
#endif

	int pow(int pow) {
		int ret = 1;
		for (int i = 0; i < pow; ++i) ret *= 2;
		return ret;
	}

	int getPower2(int i) {
		for (int power = 0;; ++power)
			if (pow(power) >= i) return pow(power);
	}

	void convertImageToPow2(Graphics4::Image::Format format, u8* from, int fw, int fh, u8* to, int tw, int th) {
		switch (format) {
		case Graphics4::Image::RGBA32:
			for (int y = 0; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					to[tw * 4 * y + x * 4 + 0] = 0;
					to[tw * 4 * y + x * 4 + 1] = 0;
					to[tw * 4 * y + x * 4 + 2] = 0;
					to[tw * 4 * y + x * 4 + 3] = 0;
				}
			}
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					to[tw * 4 * y + x * 4 + 0] = from[y * fw * 4 + x * 4 + 0];
					to[tw * 4 * y + x * 4 + 1] = from[y * fw * 4 + x * 4 + 1];
					to[tw * 4 * y + x * 4 + 2] = from[y * fw * 4 + x * 4 + 2];
					to[tw * 4 * y + x * 4 + 3] = from[y * fw * 4 + x * 4 + 3];
				}
			}
			break;
		case Graphics4::Image::Grey8:
			for (int y = 0; y < th; ++y) {
				for (int x = 0; x < tw; ++x) {
					to[tw * y + x] = 0;
				}
			}
			for (int y = 0; y < fh; ++y) {
				for (int x = 0; x < fw; ++x) {
					to[tw * y + x] = from[y * fw + x];
				}
			}
			break;
			case Graphics4::Image::RGB24:
			case Graphics4::Image::RGBA128:
			case Graphics4::Image::RGBA64:
			case Graphics4::Image::A32:
				break;
		}
	}
}

void Graphics4::Texture::init(const char* format, bool readable) {
	bool toPow2;
	if (Graphics4::nonPow2TexturesSupported()) {
		texWidth = width;
		texHeight = height;
		toPow2 = false;
	}
	else {
		texWidth = getPower2(width);
		texHeight = getPower2(height);
		toPow2 = !(texWidth == width && texHeight == height);
	}

	u8* conversionBuffer = nullptr;

	if (compressed) {
#if defined(SYS_IOS)
		texWidth = Kore::max(texWidth, texHeight);
		texHeight = Kore::max(texWidth, texHeight);
		if (texWidth < 8) texWidth = 8;
		if (texHeight < 8) texHeight = 8;
#elif defined(SYS_ANDROID)
		texWidth = width;
		texHeight = height;
#endif
	}
	else if (toPow2) {
		conversionBuffer = new u8[texWidth * texHeight * sizeOf(this->format)];
		convertImageToPow2(this->format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
	}

#ifdef SYS_ANDROID
	external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();

	int convertedType = convertType(this->format);
	bool isHdr = convertedType == GL_FLOAT;

	if (compressed) {
#if defined(SYS_IOS)
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, texWidth, texHeight, 0, texWidth * texHeight / 2, data);
//#elif defined(SYS_ANDROID)
//		u8 blockX = internalFormat >> 8;
//		u8 blockY = internalFormat & 0xff;
//		glCompressedTexImage2D(GL_TEXTURE_2D, 0, astcFormat(blockX, blockY), texWidth, texHeight, 0, dataSize, data);
#endif
	}
	else {
		void *texdata = data;
		if (isHdr) texdata = hdrData;
		else if (toPow2) texdata = conversionBuffer;
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(this->format), texWidth, texHeight, 0, convertFormat(this->format), convertedType, texdata);
		glCheckErrors();
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();

	if (toPow2) {
		delete[] conversionBuffer;
		conversionBuffer = nullptr;
	}

	if (!readable) {
		if (isHdr) {
			delete[] hdrData;
			hdrData = nullptr;
		}
		else {
			delete[] data;
			data = nullptr;
		}
	}

	if (readable && compressed) {
		log(Kore::Warning, "Compressed images can not be readable.");
	}
}

Graphics4::Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
#ifdef SYS_IOS
	texWidth = width;
	texHeight = height;
#else
	if (Graphics4::nonPow2TexturesSupported()) {
		texWidth = width;
		texHeight = height;
	}
	else {
		texWidth = getPower2(width);
		texHeight = getPower2(height);
	}
#endif
// conversionBuffer = new u8[texWidth * texHeight * 4];

#ifdef SYS_ANDROID
	external_oes = false;
#endif

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glCheckErrors();
	glGenTextures(1, &texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();

	if (convertType(format) == GL_FLOAT) {
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(format), texWidth, texHeight, 0, convertFormat(format), GL_FLOAT, nullptr);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, convertInternalFormat(format), texWidth, texHeight, 0, convertFormat(format), GL_UNSIGNED_BYTE, data);
	}
	glCheckErrors();

	/*if (!readable) {
	    delete[] data;
	    data = nullptr;
	}*/
}

Graphics4::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {
#ifndef OPENGLES
	glGenTextures(1, &texture);
	glCheckErrors();
	glBindTexture(GL_TEXTURE_3D, texture);
	glCheckErrors();

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glCheckErrors();
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glCheckErrors();

	glTexImage3D(GL_TEXTURE_3D, 0, convertInternalFormat(format), width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glCheckErrors();
#endif
}

#ifdef SYS_ANDROID
Graphics4::Texture::Texture(unsigned texid) : Image(1023, 684, Image::RGBA32, false) {
	texture = texid;
	external_oes = true;
	texWidth = 1023;
	texHeight = 684;
}
#endif

TextureImpl::~TextureImpl() {
	glDeleteTextures(1, &texture);
	glFlush();
}

void Graphics4::Texture::_set(TextureUnit unit) {
	GLenum target = depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glActiveTexture(GL_TEXTURE0 + unit.unit);
	glCheckErrors();
#ifdef SYS_ANDROID
	if (external_oes) {
		glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
		glCheckErrors();
	}
	else {
		glBindTexture(target, texture);
		glCheckErrors();
	}
#else
	glBindTexture(target, texture);
	glCheckErrors();
#endif
}

void Graphics4::Texture::_setImage(TextureUnit unit) {
#if defined(SYS_WINDOWS)
// || (defined(SYS_LINUX) && defined(GL_VERSION_4_2)) // Undefined reference on Travis
	glBindImageTexture(unit.unit, texture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glCheckErrors();
#endif
}

int Graphics4::Texture::stride() {
	return width * sizeOf(format);
}

u8* Graphics4::Texture::lock() {
	return (u8*)data;
}

/*void Texture::unlock() {
    if (conversionBuffer != nullptr) {
        convertImageToPow2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
        glBindTexture(GL_TEXTURE_2D, texture);
#ifndef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA :
GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
    }
}*/

void Graphics4::Texture::unlock() {
	// if (conversionBuffer != nullptr) {
	// convertImageToPow2(format, (u8*)data, width, height, conversionBuffer, texWidth, texHeight);
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	// glTexImage2D(GL_TEXTURE_2D, 0, (format == Image::RGBA32) ? GL_RGBA : GL_LUMINANCE, texWidth, texHeight, 0, (format == Image::RGBA32) ? GL_RGBA :
	// GL_LUMINANCE, GL_UNSIGNED_BYTE, conversionBuffer);
	if (convertType(format) == GL_FLOAT) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, convertFormat(format), GL_FLOAT, hdrData);
		glCheckErrors();
	}
	else {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, convertFormat(format), GL_UNSIGNED_BYTE, data);
		glCheckErrors();
	}
	// }
}

void Graphics4::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {
#ifdef GL_VERSION_4_4
	static float clearColor[4];
	clearColor[0] = ((color & 0x00ff0000) >> 16) / 255.0f;
	clearColor[1] = ((color & 0x0000ff00) >> 8) / 255.0f;
	clearColor[2] = (color & 0x000000ff) / 255.0f;
	clearColor[3] = ((color & 0xff000000) >> 24) / 255.0f;
	GLenum target = depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture);
	glClearTexSubImage(texture, 0, x, y, z, width, height, depth, convertFormat(format), convertType(format), clearColor);
#endif
}

#ifdef SYS_IOS
void Graphics4::Texture::upload(u8* data) {
	glBindTexture(GL_TEXTURE_2D, texture);
	glCheckErrors();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, convertFormat(format), GL_UNSIGNED_BYTE, data);
	glCheckErrors();
}
#endif

void Graphics4::Texture::generateMipmaps(int levels) {
	GLenum target = depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture);
	glCheckErrors();
	glGenerateMipmap(target);
	glCheckErrors();
}

void Graphics4::Texture::setMipmap(Texture* mipmap, int level) {
	int convertedType = convertType(mipmap->format);
	bool isHdr = convertedType == GL_FLOAT;
	GLenum target = depth > 1 ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	glBindTexture(target, texture);
	glCheckErrors();
	if (isHdr) {
		glTexImage2D(target, level, convertInternalFormat(mipmap->format), mipmap->texWidth, mipmap->texHeight, 0, convertFormat(mipmap->format), convertedType,
		             mipmap->hdrData);
		glCheckErrors();
	}
	else {
		glTexImage2D(target, level, convertInternalFormat(mipmap->format), mipmap->texWidth, mipmap->texHeight, 0, convertFormat(mipmap->format), convertedType,
		             mipmap->data);
		glCheckErrors();
	}
}