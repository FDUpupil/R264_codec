#ifndef __RAW_IMAGE_H__
#define __RAW_IMAGE_H__

#include "common/type.h"

#include <memory>

enum RawImageFormat {
    RAW_IMAGE_PLANAR_FORMAT,
    RAW_IMAGE_PACKED_FORMAT,
    RAW_IMAGE_SEMI_PLANAR_FORMAT,
    RAW_IMAGE_FORMAT_COUNT
};

class RawImage {
public:
    RawImage(RawImageFormat _imageFormat, ChromaArrayType _chromaFormat, uint8_t _bitDepth, bool _bigEndian, bool _cbFirst, uint16_t _width, uint16_t _height);
    ~RawImage();

    void require(std::unique_ptr<uint8_t []> buf);
    std::unique_ptr<uint8_t []> release();

    ChromaArrayType getChromaFormat() const { return chromaFormat; }
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
    size_t getFrameSize() const { return frameSize; }
    void setPosition(uint16_t x, uint16_t y);
    PixType readPixel(ColourComponent planeID) { return (this->*readFunc)(planeID); }
    void writePixel(ColourComponent planeID, PixType value) { (this->*writeFunc)(planeID, value); }

private:
    ChromaArrayType chromaFormat;
    uint8_t bitDepth;
    uint16_t width;
    uint16_t height;
    size_t frameSize;

    std::unique_ptr<uint8_t []> rawBuf;
    uint8_t *comp[COLOUR_COMPONENT_COUNT];
    size_t offset[COLOUR_COMPONENT_COUNT];
    uint8_t step[COLOUR_COMPONENT_COUNT];

    PixType (RawImage::*readFunc)(ColourComponent planeID);
    void (RawImage::*writeFunc)(ColourComponent planeID, PixType value);

    void setImageInfo(RawImageFormat _imageFormat, ChromaArrayType _chromaFormat, uint8_t _bitDepth, bool _bigEndian, bool _cbFirst, uint16_t _width, uint16_t _height);

    PixType read8(ColourComponent planeID);
    PixType read16LE(ColourComponent planeID);
    PixType read16BE(ColourComponent planeID);

    void write8(ColourComponent planeID, PixType value);
    void write16LE(ColourComponent planeID, PixType value);
    void write16BE(ColourComponent planeID, PixType value);
};

#endif
