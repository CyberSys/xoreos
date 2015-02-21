/* xoreos - A reimplementation of BioWare's Aurora engine
 *
 * xoreos is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * xoreos is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * xoreos is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xoreos. If not, see <http://www.gnu.org/licenses/>.
 */

// Partially based on the TGA implementation found in ScummVM.

/** @file
 *  Decoding TGA (TarGa) images.
 */

#include "src/common/util.h"
#include "src/common/stream.h"
#include "src/common/error.h"

#include "src/graphics/util.h"
#include "src/graphics/images/tga.h"

namespace Graphics {

TGA::TGA(Common::SeekableReadStream &tga) {
	_compressed = false;

	load(tga);
}

TGA::~TGA() {
}

void TGA::load(Common::SeekableReadStream &tga) {
	try {

		ImageType imageType;
		byte pixelDepth, imageDesc;
		readHeader(tga, imageType, pixelDepth, imageDesc);
		readData  (tga, imageType, pixelDepth, imageDesc);

		if (tga.err())
			throw Common::Exception(Common::kReadError);

	} catch (Common::Exception &e) {
		clear();

		e.add("Failed reading TGA file");
		throw;
	}
}

void TGA::readHeader(Common::SeekableReadStream &tga, ImageType &imageType, byte &pixelDepth, byte &imageDesc) {
	if (!tga.seek(0))
		throw Common::Exception(Common::kSeekError);

	// TGAs have an optional "id" string in the header
	uint32 idLength = tga.readByte();

	// Number of colors in the color map / palette
	if (tga.readByte() != 0)
		throw Common::Exception("Unsupported feature: Color map");

	// Image type. 2 == unmapped RGB, 3 == Grayscale
	imageType = (ImageType)tga.readByte();
	if (!isSupportedImageType(imageType))
		throw Common::Exception("Unsupported image type: %d", imageType);

	// Color map specifications + X + Y
	tga.skip(5 + 2 + 2);

	_mipMaps.push_back(new MipMap(this));

	// Image dimensions
	_mipMaps[0]->width  = tga.readUint16LE();
	_mipMaps[0]->height = tga.readUint16LE();

	// Bits per pixel
	pixelDepth = tga.readByte();

	if (imageType == kImageTypeTrueColor || imageType == kImageTypeRLETrueColor) {
		if (pixelDepth == 24) {
			_hasAlpha  = false;
			_format    = kPixelFormatBGR;
			_formatRaw = kPixelFormatRGB8;
			_dataType  = kPixelDataType8;
		} else if (pixelDepth == 16 || pixelDepth == 32) {
			_hasAlpha  = true;
			_format    = kPixelFormatBGRA;
			_formatRaw = kPixelFormatRGBA8;
			_dataType  = kPixelDataType8;
		} else
			throw Common::Exception("Unsupported pixel depth: %d, %d", imageType, pixelDepth);
	} else if (imageType == kImageTypeBW) {
		if (pixelDepth != 8)
			throw Common::Exception("Unsupported pixel depth: %d, %d", imageType, pixelDepth);

		_hasAlpha  = false;
		_format    = kPixelFormatBGRA;
		_formatRaw = kPixelFormatRGBA8;
		_dataType  = kPixelDataType8;
	}

	// Image descriptor
	imageDesc = tga.readByte();

	// Skip the id string
	tga.skip(idLength);
}

void TGA::readData(Common::SeekableReadStream &tga, ImageType imageType, byte pixelDepth, byte imageDesc) {
	if (imageType == kImageTypeTrueColor || imageType == kImageTypeRLETrueColor) {
		_mipMaps[0]->size = _mipMaps[0]->width * _mipMaps[0]->height;
		if      (_format == kPixelFormatBGR)
			_mipMaps[0]->size *= 3;
		else if (_format == kPixelFormatBGRA)
			_mipMaps[0]->size *= 4;

		_mipMaps[0]->data = new byte[_mipMaps[0]->size];

		if (imageType == kImageTypeTrueColor) {
			if (pixelDepth == 16) {
				// Convert from 16bpp to 32bpp
				// 16bpp TGA is ARGB1555
				uint16 count = _mipMaps[0]->width * _mipMaps[0]->height;
				byte *dst = _mipMaps[0]->data;

				while (count--) {
					uint16 pixel = tga.readUint16LE();

					*dst++ = (pixel & 0x1F) << 3;
					*dst++ = (pixel & 0x3E0) >> 2;
					*dst++ = (pixel & 0x7C00) >> 7;
					*dst++ = (pixel & 0x8000) ? 0xFF : 0x00;
				}
			} else {
				// Read it in raw
				tga.read(_mipMaps[0]->data, _mipMaps[0]->size);
			}
		} else {
			readRLE(tga, pixelDepth);
		}
	} else if (imageType == kImageTypeBW) {
		_mipMaps[0]->size = _mipMaps[0]->width * _mipMaps[0]->height * 4;
		_mipMaps[0]->data = new byte[_mipMaps[0]->size];

		byte  *data  = _mipMaps[0]->data;
		uint32 count = _mipMaps[0]->width * _mipMaps[0]->height;

		while (count-- > 0) {
			byte g = tga.readByte();

			memset(data, g, 3);
			data[3] = 0xFF;

			data += 4;
		}

	}

	// Bit 5 of imageDesc set means the origin in upper-left corner
	if (imageDesc & 0x20)
		flipVertically(_mipMaps[0]->data, _mipMaps[0]->width, _mipMaps[0]->height, pixelDepth / 8);
}

void TGA::readRLE(Common::SeekableReadStream &tga, byte pixelDepth) {
	if (pixelDepth != 24 && pixelDepth != 32)
		throw Common::Exception("Unhandled RLE depth %d", pixelDepth);

	byte  *data  = _mipMaps[0]->data;
	uint32 count = _mipMaps[0]->width * _mipMaps[0]->height;

	while (count > 0) {
		byte code = tga.readByte();
		byte length = (code & 0x7F) + 1;
		count -= length;

		if (code & 0x80) {
			if (pixelDepth == 32) {
				uint32 color = tga.readUint32BE();

				while (length--) {
					WRITE_BE_UINT32(data, color);
					data += 4;
				}
			} else if (pixelDepth == 24) {
				byte b = tga.readByte();
				byte g = tga.readByte();
				byte r = tga.readByte();

				while (length--) {
					*data++ = b;
					*data++ = g;
					*data++ = r;
				}
			}
		} else {
			if (pixelDepth == 32) {
				while (length--) {
					WRITE_BE_UINT32(data, tga.readUint32BE());
					data += 4;
				}
			} else if (pixelDepth == 24) {
				while (length--) {
					*data++ = tga.readByte();
					*data++ = tga.readByte();
					*data++ = tga.readByte();
				}
			}
		}
	}
}

bool TGA::isSupportedImageType(ImageType type) const {
	// We currently only support a limited number of types
	switch (type) {
	case kImageTypeTrueColor:
	case kImageTypeBW:
	case kImageTypeRLETrueColor:
		return true;
	default:
		break;
	}

	return false;
}

} // End of namespace Graphics
