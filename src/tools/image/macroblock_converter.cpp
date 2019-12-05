#include "macroblock_converter.h"

#include <exception>
#include <stdexcept>

MacroblockConverter::MacroblockConverter(uint8_t _bitDepth, const FrameCrop &_cropInfo)
    : bitDepth(_bitDepth), cropInfo(_cropInfo)
{
    if (!cropInfo.flag) {
        cropInfo.leftOffset = 0;
        cropInfo.rightOffset = 0;
        cropInfo.topOffset = 0;
        cropInfo.bottomOffset = 0;
    }
}

void MacroblockConverter::convertImageToMacroblocks(RawImage &raw, BlockyImage &blocky) const
{
    ChromaArrayType chromaFormat = raw.getChromaFormat();
    uint16_t width = blocky.getWidth() * 16;
    uint16_t height = blocky.getHeight() * 16;
    uint16_t xStart = cropInfo.leftOffset;
    uint16_t xEnd = width - cropInfo.rightOffset;
    uint16_t yStart = cropInfo.topOffset;
    uint16_t yEnd = height - cropInfo.bottomOffset;
    uint8_t mbWC = (chromaFormat == CHROMA_FORMAT_400) ? 0 : (chromaFormat == CHROMA_FORMAT_444) ? 16 : 8;
    uint8_t mbHC = (chromaFormat == CHROMA_FORMAT_400) ? 0 : (chromaFormat != CHROMA_FORMAT_420) ? 16 : 8;

    if (raw.getChromaFormat() != blocky.getChromaFormat())
        throw std::exception(std::logic_error("Chroma-sampling format is not matched"));

    if ((raw.getWidth() + cropInfo.leftOffset + cropInfo.rightOffset != width)
        || (raw.getHeight() + cropInfo.topOffset + cropInfo.bottomOffset != height))
        throw std::exception(std::logic_error("Cropping area is not matched between raw raw and macroblocks array"));

    if (raw.getWidth() < 16 || raw.getHeight() < 16)
        throw std::exception(std::logic_error("Target resolution is less than 16x16"));

    raw.setPosition(0, 0);

    if (cropInfo.topOffset > 0) {
        fillCroppedArea(
            blocky, COLOUR_COMPONENT_Y, 16, 16,
            0, width, 0, yStart);
        if (mbWC && mbHC) {
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CB, mbWC, mbHC,
                0, width, 0, yStart);
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CR, mbWC, mbHC,
                0, width, 0, yStart);
        }
    }
    if (cropInfo.leftOffset > 0) {
        fillCroppedArea(
            blocky, COLOUR_COMPONENT_Y, 16, 16,
            0, xStart, yStart, yEnd);
        if (mbWC && mbHC) {
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CB, mbWC, mbHC,
                0, xStart, yStart, yEnd);
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CR, mbWC, mbHC,
                0, xStart, yStart, yEnd);
        }
    }
    if (cropInfo.rightOffset > 0) {
        fillCroppedArea(
            blocky, COLOUR_COMPONENT_Y, 16, 16,
            xEnd, width, yStart, yEnd);
        if (mbWC && mbHC) {
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CB, mbWC, mbHC,
                xEnd, width, yStart, yEnd);
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CR, mbWC, mbHC,
                xEnd, width, yStart, yEnd);
        }
    }
    if (cropInfo.bottomOffset > 0) {
        fillCroppedArea(
            blocky, COLOUR_COMPONENT_Y, 16, 16,
            0, width, yEnd, height);
        if (mbWC && mbHC) {
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CB, mbWC, mbHC,
                0, width, yEnd, height);
            fillCroppedArea(
                blocky, COLOUR_COMPONENT_CR, mbWC, mbHC,
                0, width, yEnd, height);
        }
    }

    copyDisplayArea(
        raw, blocky, COLOUR_COMPONENT_Y, 16, 16,
        xStart, xEnd, yStart, yEnd);
    if (mbWC && mbHC) {
        copyDisplayArea(
            raw, blocky, COLOUR_COMPONENT_CB, mbWC, mbHC,
            xStart, xEnd, yStart, yEnd);
        copyDisplayArea(
            raw, blocky, COLOUR_COMPONENT_CR, mbWC, mbHC,
            xStart, xEnd, yStart, yEnd);
    }
}

void MacroblockConverter::convertMacroblocksToImage(const BlockyImage &blocky, RawImage &raw) const
{
    ChromaArrayType chromaFormat = raw.getChromaFormat();
    uint16_t width = blocky.getWidth() * 16;
    uint16_t height = blocky.getHeight() * 16;
    uint16_t xStart = cropInfo.leftOffset;
    uint16_t xEnd = width - cropInfo.rightOffset;
    uint16_t yStart = cropInfo.topOffset;
    uint16_t yEnd = height - cropInfo.bottomOffset;
    uint8_t mbWC = (chromaFormat == CHROMA_FORMAT_400) ? 0 : (chromaFormat == CHROMA_FORMAT_444) ? 16 : 8;
    uint8_t mbHC = (chromaFormat == CHROMA_FORMAT_400) ? 0 : (chromaFormat != CHROMA_FORMAT_420) ? 16 : 8;

    if ((raw.getWidth() + cropInfo.leftOffset + cropInfo.rightOffset != width)
        || (raw.getHeight() + cropInfo.topOffset + cropInfo.bottomOffset != height))
        throw std::exception(std::logic_error("Cropping area is not matched between raw raw and macroblocks array"));

    if (raw.getWidth() < 16 || raw.getHeight() < 16)
        throw std::exception(std::logic_error("Target resolution is less than 16x16"));

    raw.setPosition(0, 0);

    copyDisplayArea(
        blocky, raw, COLOUR_COMPONENT_Y, 16, 16,
        xStart, xEnd, yStart, yEnd);
    if (mbWC && mbHC) {
        copyDisplayArea(
            blocky, raw, COLOUR_COMPONENT_CB, mbWC, mbHC,
            xStart, xEnd, yStart, yEnd);
        copyDisplayArea(
            blocky, raw, COLOUR_COMPONENT_CR, mbWC, mbHC,
            xStart, xEnd, yStart, yEnd);
    }
}

void MacroblockConverter::fillCroppedArea(
    BlockyImage &blocky, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
    uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const
{
    PixType val = 1 << (bitDepth - 1);
    for (uint16_t yInMbs = yStart / 16; yInMbs < (yEnd + 15) / 16; ++yInMbs) {
        Macroblock *curMb = &blocky[yStart / 16][xStart / 16];
        for (uint16_t xInMbs = xStart / 16; xInMbs < (xEnd + 15) / 16; ++xInMbs) {
            for (uint16_t yOfMbs = 0; yOfMbs < mbHeight; ++yOfMbs)
                for (uint16_t xOfMbs = 0; xOfMbs < mbWidth; ++xOfMbs)
                    (*curMb)[planeID][yOfMbs][xOfMbs] = val;
            ++curMb;
        }
    }
}

void MacroblockConverter::copyDisplayArea(
    RawImage &raw, BlockyImage &blocky, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
    uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const
{
    if (mbWidth == 8) {
        xEnd = xStart / 2 + (xEnd - xStart + 1) / 2;
        xStart = xStart / 2;
    }
    if (mbHeight == 8) {
        yEnd = yStart / 2 + (yEnd - yStart + 1) / 2;
        yStart = yStart / 2;
    }

    for (uint16_t y = yStart; y < yEnd; ++y) {
        uint16_t yInMbs = y / mbHeight;
        uint16_t yOfMbs = y % mbHeight;
        uint16_t xInMbs = xStart / mbWidth;
        uint16_t xOfMbs = 0;
        Macroblock *curMb = &blocky[yInMbs][xInMbs];

        if (xStart % mbWidth != 0) {
            for (xOfMbs = xStart % mbWidth; xOfMbs < mbWidth; ++xOfMbs)
                (*curMb)[planeID][yOfMbs][xOfMbs] = raw.readPixel(planeID);
            ++xInMbs;
            ++curMb;
        }

        for (; xInMbs < xEnd / mbWidth; ++xInMbs) {
            for (xOfMbs = 0; xOfMbs < mbWidth; ++xOfMbs)
                (*curMb)[planeID][yOfMbs][xOfMbs] = raw.readPixel(planeID);
            ++curMb;
        }

        if (xEnd % mbWidth != 0) {
            for (xOfMbs = 0; xOfMbs < xEnd % mbWidth; ++xOfMbs)
                (*curMb)[planeID][yOfMbs][xOfMbs] = raw.readPixel(planeID);
        }
    }
}

void MacroblockConverter::copyDisplayArea(
    const BlockyImage &blocky, RawImage &raw, ColourComponent planeID, uint8_t mbWidth, uint8_t mbHeight,
    uint16_t xStart, uint16_t xEnd, uint16_t yStart, uint16_t yEnd) const
{
    if (mbWidth == 8) {
        xEnd = xStart / 2 + (xEnd - xStart + 1) / 2;
        xStart = xStart / 2;
    }
    if (mbHeight == 8) {
        yEnd = yStart / 2 + (yEnd - yStart + 1) / 2;
        yStart = yStart / 2;
    }

    for (uint16_t y = yStart; y < yEnd; ++y) {
        uint16_t yInMbs = y / mbHeight;
        uint16_t yOfMbs = y % mbHeight;
        uint16_t xInMbs = xStart / mbWidth;
        uint16_t xOfMbs = 0;
        const Macroblock *curMb = &blocky[yInMbs][xInMbs];

        if (xStart % mbWidth != 0) {
            for (xOfMbs = xStart % mbWidth; xOfMbs < mbWidth; ++xOfMbs)
                raw.writePixel(planeID, (*curMb)[planeID][yOfMbs][xOfMbs]);
            ++xInMbs;
            ++curMb;
        }

        for (; xInMbs < xEnd / mbWidth; ++xInMbs) {
            for (xOfMbs = 0; xOfMbs < mbWidth; ++xOfMbs)
                raw.writePixel(planeID, (*curMb)[planeID][yOfMbs][xOfMbs]);
            ++curMb;
        }

        if (xEnd % mbWidth != 0) {
            for (xOfMbs = 0; xOfMbs < xEnd % mbWidth; ++xOfMbs)
                raw.writePixel(planeID, (*curMb)[planeID][yOfMbs][xOfMbs]);
        }
    }
}
