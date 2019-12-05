#ifndef __MACROBLOCK_CONVERTER_H__
#define __MACROBLOCK_CONVERTER_H__

#include "common/type.h"
#include "raw_image.h"
#include "blocky_image.h"

class MacroblockConverter {
public:
    MacroblockConverter(uint8_t _bitDepth, const FrameCrop &_cropInfo);

    void convertImageToMacroblocks(RawImage &raw, BlockyImage &blocky) const;
    void convertMacroblocksToImage(const BlockyImage &blocky, RawImage &raw) const;

private:
    uint8_t bitDepth;
    FrameCrop cropInfo;

    void fillCroppedArea(
        BlockyImage &blocky, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
        uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const;
    void copyDisplayArea(
        RawImage &raw, BlockyImage &blocky, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
        uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const;
    void copyDisplayArea(
        const BlockyImage &blocky, RawImage &raw, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
        uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const;
};

#endif
