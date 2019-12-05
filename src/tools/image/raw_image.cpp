#include "raw_image.h"

#include <memory>
#include <utility>
#include <exception>
#include <stdexcept>

RawImage::RawImage(RawImageFormat _imageFormat, ChromaArrayType _chromaFormat, uint8_t _bitDepth, bool _bigEndian, bool _cbFirst, uint16_t _width, uint16_t _height)
    : chromaFormat(_chromaFormat), bitDepth(_bitDepth), width(_width), height(_height)
{
    comp[COLOUR_COMPONENT_Y]  = nullptr;
    comp[COLOUR_COMPONENT_CB] = nullptr;
    comp[COLOUR_COMPONENT_CR] = nullptr;

    setImageInfo(_imageFormat, _chromaFormat, _bitDepth, _bigEndian, _cbFirst, _width, _height);
}

RawImage::~RawImage()
{
    rawBuf.reset();
}

void RawImage::require(std::unique_ptr<uint8_t []> buf)
{
    rawBuf = std::move(buf);

    comp[COLOUR_COMPONENT_Y]  = &rawBuf[offset[COLOUR_COMPONENT_Y]];
    comp[COLOUR_COMPONENT_CB] = &rawBuf[offset[COLOUR_COMPONENT_CB]];
    comp[COLOUR_COMPONENT_CR] = &rawBuf[offset[COLOUR_COMPONENT_CR]];
}

std::unique_ptr<uint8_t []> RawImage::release()
{
    return std::move(rawBuf);
}

void RawImage::setPosition(uint16_t x, uint16_t y)
{
    uint8_t pixSize;
    int lumaOffset;
    int chromaOffset;

    if (x >= width || y >= height)
        throw std::exception(std::logic_error("Position is out of bound"));
    
    pixSize = (bitDepth > 8) ? 2 : 1;
    lumaOffset = (y * width + x) * pixSize;
    switch (chromaFormat) {
    case CHROMA_FORMAT_420:
        chromaOffset = ((y / 2) * ((width + 1) / 2) + (x / 2)) * pixSize;
        break;

    case CHROMA_FORMAT_422:
        chromaOffset = (y * ((width + 1) / 2) + (x / 2)) * pixSize;
        break;

    case CHROMA_FORMAT_444:
        chromaOffset = (y * width + x) * pixSize;
        break;

    default:
        chromaOffset = 0;
    }

    comp[COLOUR_COMPONENT_Y]  = &rawBuf[offset[COLOUR_COMPONENT_Y]  + lumaOffset];
    comp[COLOUR_COMPONENT_CB] = &rawBuf[offset[COLOUR_COMPONENT_CB] + chromaOffset];
    comp[COLOUR_COMPONENT_CR] = &rawBuf[offset[COLOUR_COMPONENT_CR] + chromaOffset];
}

/**
 * ChromaArrayFormat = 4:0:0
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * 
 * ChromaArrayFormat = 4:2:0
 * Planar:
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * |-------|
 * | Cb/Cr |
 * |-------|
 * |-------|
 * | Cr/Cb |
 * |-------|
 * 
 * Packed: (not available)
 * 
 * Semi-Planar:
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * |---------------|
 * |Cb|Cr|Cb|Cr|...|
 * |---------------|
 * 
 * ChromaArrayFormat = 4:2:2
 * Planar:
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * |-------|
 * |       |
 * | Cb/Cr |
 * |       |
 * |-------|
 * |-------|
 * |       |
 * | Cr/Cb |
 * |       |
 * |-------|
 * 
 * Packed:
 * |-------------|
 * |Y|Cb|Y|Cr|...|
 * |Y|Cb|Y|Cr|...|
 * |Y|Cb|Y|Cr|...|
 * |-------------|
 * 
 * Semi-Planar:
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * |---------------|
 * |Cb|Cr|Cb|Cr|...|
 * |Cb|Cr|Cb|Cr|...|
 * |Cb|Cr|Cb|Cr|...|
 * |---------------|
 * 
 * ChromaArrayFormat = 4:2:2
 * Planar:
 * |---------------|
 * |               |
 * |       Y       |
 * |               |
 * |---------------|
 * |---------------|
 * |               |
 * |     Cb/Cr     |
 * |               |
 * |---------------|
 * |---------------|
 * |               |
 * |     Cr/Cb     |
 * |               |
 * |---------------|
 * 
 * Packed:
 * |-------------------|
 * |Y|Cb|Cr|Y|Cb|Cr|...|
 * |Y|Cb|Cr|Y|Cb|Cr|...|
 * |Y|Cb|Cr|Y|Cb|Cr|...|
 * |-------------------|
 * 
 * Semi-Planar: (not available)
 */

void RawImage::setImageInfo(RawImageFormat _imageFormat, ChromaArrayType _chromaFormat, uint8_t _bitDepth, bool _bigEndian, bool _cbFirst, uint16_t _width, uint16_t _height)
{
    uint8_t pixSize = (_bitDepth > 8) ? 2 : 1;

    if (_chromaFormat == CHROMA_FORMAT_400) {
        offset[COLOUR_COMPONENT_Y]  = 0;
        offset[COLOUR_COMPONENT_CB] = 0;
        offset[COLOUR_COMPONENT_CR] = 0;

        step[COLOUR_COMPONENT_Y]  = 1 * pixSize;
        step[COLOUR_COMPONENT_CB] = 0;
        step[COLOUR_COMPONENT_CR] = 0;

        frameSize = _width * _height * pixSize;
    } else {
        int widthC  = (_chromaFormat == CHROMA_FORMAT_444) ? _width  : (_width  + 1) / 2;
        int heightC = (_chromaFormat != CHROMA_FORMAT_420) ? _height : (_height + 1) / 2;
        size_t chromaCompOffset[2];

        switch (_imageFormat) {
        case RAW_IMAGE_PLANAR_FORMAT:
            offset[COLOUR_COMPONENT_Y] = 0;
            chromaCompOffset[0] = _width * _height * pixSize;
            chromaCompOffset[1] = chromaCompOffset[0] + widthC * heightC * pixSize;

            step[COLOUR_COMPONENT_Y]  = 1 * pixSize;
            step[COLOUR_COMPONENT_CB] = 1 * pixSize;
            step[COLOUR_COMPONENT_CR] = 1 * pixSize;
            break;

        case RAW_IMAGE_PACKED_FORMAT:
            if (_chromaFormat == CHROMA_FORMAT_422) {
                offset[COLOUR_COMPONENT_Y] = 0;
                chromaCompOffset[0] = 1 * pixSize;
                chromaCompOffset[1] = 3 * pixSize;

                step[COLOUR_COMPONENT_Y]  = 2 * pixSize;
                step[COLOUR_COMPONENT_CB] = 4 * pixSize;
                step[COLOUR_COMPONENT_CR] = 4 * pixSize;
            } else if (_chromaFormat == CHROMA_FORMAT_444) {
                offset[COLOUR_COMPONENT_Y] = 0;
                chromaCompOffset[0] = 1 * pixSize;
                chromaCompOffset[1] = 2 * pixSize;

                step[COLOUR_COMPONENT_Y]  = 3 * pixSize;
                step[COLOUR_COMPONENT_CB] = 3 * pixSize;
                step[COLOUR_COMPONENT_CR] = 3 * pixSize;
            } else
                goto fail;
            break;

        case RAW_IMAGE_SEMI_PLANAR_FORMAT:
            if (_chromaFormat == CHROMA_FORMAT_420 || _chromaFormat == CHROMA_FORMAT_422) {
                offset[COLOUR_COMPONENT_Y] = 0;
                chromaCompOffset[0] = _width * _height * pixSize;
                chromaCompOffset[1] = (_width * _height + 1) * pixSize;

                step[COLOUR_COMPONENT_Y]  = 1 * pixSize;
                step[COLOUR_COMPONENT_CB] = 2 * pixSize;
                step[COLOUR_COMPONENT_CR] = 2 * pixSize;
            } else
                goto fail;
            break;

        default:
            goto fail;
        }

        if (_cbFirst) {
            offset[COLOUR_COMPONENT_CB] = chromaCompOffset[0];
            offset[COLOUR_COMPONENT_CR] = chromaCompOffset[1];
        } else {
            offset[COLOUR_COMPONENT_CB] = chromaCompOffset[1];
            offset[COLOUR_COMPONENT_CR] = chromaCompOffset[0];
        }

        frameSize = (_width * _height + 2 * widthC * heightC) * pixSize;
    }

    if (pixSize == 1) {
        readFunc = &RawImage::read8;
        writeFunc = &RawImage::write8;
    } else if (_bigEndian) {
        readFunc = &RawImage::read16BE;
        writeFunc = &RawImage::write16BE;
    } else {
        readFunc = &RawImage::read16LE;
        writeFunc = &RawImage::write16LE;
    }
    
    return;

fail:
    throw std::exception(std::logic_error("Unsupported image format"));
}

PixType RawImage::read8(ColourComponent planeID)
{
    PixType ret;
    ret = *comp[planeID];
    comp[planeID] += step[planeID];
    return ret;
}

PixType RawImage::read16LE(ColourComponent planeID)
{
    PixType ret;
    ret = *comp[planeID];
    ret |= *(comp[planeID] + 1) << 8;
    comp[planeID] += step[planeID];
    return ret;
}

PixType RawImage::read16BE(ColourComponent planeID)
{
    PixType ret;
    ret = *comp[planeID] << 8;
    ret |= *(comp[planeID] + 1);
    comp[planeID] += step[planeID];
    return ret;
}

void RawImage::write8(ColourComponent planeID, PixType value)
{
    *comp[planeID] = (uint8_t)value;
    comp[planeID] += step[planeID];
}

void RawImage::write16LE(ColourComponent planeID, PixType value)
{
    *comp[planeID] = (uint8_t)value;
    *(comp[planeID] + 1) = (uint8_t)(value >> 8);
    comp[planeID] += step[planeID];
}

void RawImage::write16BE(ColourComponent planeID, PixType value)
{
    *comp[planeID] = (uint8_t)(value >> 8);
    *(comp[planeID] + 1) = (uint8_t)value;
    comp[planeID] += step[planeID];
}
