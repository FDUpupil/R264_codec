#ifndef __CABAC_H__
#define __CABAC_H__

#include "type.h"
#include "math.h"

#include <cassert>

#define CABACCheck(expr) assert(expr)

const int CABAC_CONTEXT_MODELS_COUNT = 1024;
const int CABAC_CONTEXT_STATES_COUNT = 64;

enum CABACCodingMode {
    CABAC_DECISION,
    CABAC_BYPASS,
    CABAC_TERMINATE
};

class ContextModelSet {
public:
    void init(int8_t sliceQP, int8_t cabacInitIdc = -1);
    void update(uint16_t ctxIdx, uint8_t binVal);
    uint8_t getRangeLPS(uint16_t ctxIdx, uint16_t range) const;
    uint8_t getValMPS(uint16_t ctxIdx) const;

    static uint16_t getCtxIdxForMbType(SliceType type, const CordTermFlag2D<1> &flag, uint8_t binIdx);
    static uint16_t getCtxIdxForMbQPDelta(const uint8_t &flag, uint8_t binIdx);
    static uint16_t getCtxIdxForIntraChromaPredMode(const CordTermFlag2D<1> &flag, uint8_t binIdx);
    static uint16_t getCtxIdxForPrevIntraNxNPredModeFlag() { return 68; }
    static uint16_t getCtxIdxForRemIntraNxNPredMode() { return 69; }
    static uint16_t getCtxIdxForCodedBlockPatternLuma(const CordTermFlag2D<1> &flag);
    static uint16_t getCtxIdxForCodedBlockPatternChroma(uint8_t binIdx, const CordTermFlag2D<1> &flag);
    static uint16_t getCtxIdxForTransformSize8x8Flag(const CordTermFlag2D<1> &flag);
    static uint16_t getCtxIdxForCodedBlockFlag(ColourComponent ctxID, CodedBlockType type, const CordTermFlag2D<1> &flag);
    static uint16_t getCtxIdxOffsetForSignificantCoeffFlag(ColourComponent ctxID, CodedBlockType type);
    static uint8_t getCtxIdxIncForSignificantCoeffFlag(ChromaArrayType chromaFormat, CodedBlockType type, uint8_t binIdx);
    static uint16_t getCtxIdxOffsetForLastSignificantCoeffFlag(ColourComponent ctxID, CodedBlockType type);
    static uint8_t getCtxIdxIncForLastSignificantCoeffFlag(ChromaArrayType chromaFormat, CodedBlockType type, uint8_t binIdx);
    static uint16_t getCtxIdxOffsetForCoeffAbsLevelMinus1(ColourComponent ctxID, CodedBlockType type);
    static uint8_t updateCtxIdxIncForCoeffAbsLevelMinus1(CodedBlockType type, uint8_t ctxIdxInc, uint32_t codeNum);

private:
    struct ContextModel {
        uint8_t pStateIdx;
        uint8_t valMPS;
    };

    struct ContextModelInitPair {
        int8_t m;
        int8_t n;
    };

    static const ContextModelInitPair ctxInitTable[CABAC_CONTEXT_MODELS_COUNT][4];
    static const uint8_t rangeTabLPS[CABAC_CONTEXT_STATES_COUNT][4];
    static const uint8_t transIdxLPS[CABAC_CONTEXT_STATES_COUNT];
    static const uint8_t transIdxMPS[CABAC_CONTEXT_STATES_COUNT];

    ContextModel ctxModels[CABAC_CONTEXT_MODELS_COUNT];
};

inline uint16_t ContextModelSet::getCtxIdxForMbType(SliceType type, const CordTermFlag2D<1> &flag, uint8_t binIdx)
{
    switch (type) {
    case I_SLICE:
        if (binIdx == 0)
            return 3 + flag.a + flag.b;
        else if (binIdx == 1)
            return 276;
        else
            return 3 + binIdx + 1;

    default:
        CABACCheck(false);
    }

    return 0xFFFF;
}

inline uint16_t ContextModelSet::getCtxIdxForMbQPDelta(const uint8_t &flag, uint8_t binIdx)
{
    if (binIdx == 0)
        return 60 + flag;
    else
        return 60 + 2;
}

inline uint16_t ContextModelSet::getCtxIdxForIntraChromaPredMode(const CordTermFlag2D<1> &flag, uint8_t binIdx)
{
    if (binIdx == 0)
        return 64 + flag.a + flag.b;
    else
        return 64 + 3;
}

inline uint16_t ContextModelSet::getCtxIdxForCodedBlockPatternLuma(const CordTermFlag2D<1> &flag)
{
    return 76 - flag.a - 2 * flag.b;
}

inline uint16_t ContextModelSet::getCtxIdxForCodedBlockPatternChroma(uint8_t binIdx, const CordTermFlag2D<1> &flag)
{
    return 77 + flag.a + 2 * flag.b + 4 * binIdx;
}

inline uint16_t ContextModelSet::getCtxIdxForTransformSize8x8Flag(const CordTermFlag2D<1> &flag)
{
    return 399 + flag.a + flag.b;
}

inline uint16_t ContextModelSet::getCtxIdxForCodedBlockFlag(ColourComponent ctxID, CodedBlockType type, const CordTermFlag2D<1> &flag)
{
    static uint16_t ctxIdxOffsetForCodedBlockFlag[COLOUR_COMPONENT_COUNT][CODED_BLOCK_TYPE_COUNT] = {
        //       Luma         |      Chroma
        // DC   AC  4x4   8x8 |    DC      AC
        {  85,  89,  93, 1012, 0xFFFF, 0xFFFF }, // Y
        { 460, 464, 468, 1016,     97,    101 }, // Cb
        { 472, 476, 480, 1020,     97,    101 }  // Cr
    };

    CABACCheck(ctxIdxOffsetForCodedBlockFlag[ctxID][type] < CABAC_CONTEXT_MODELS_COUNT);
    return ctxIdxOffsetForCodedBlockFlag[ctxID][type] + flag.a + 2 * flag.b;
}

inline uint16_t ContextModelSet::getCtxIdxOffsetForSignificantCoeffFlag(ColourComponent ctxID, CodedBlockType type)
{
    static uint16_t ctxIdxOffsetForSignificantCoeffFlag[COLOUR_COMPONENT_COUNT][CODED_BLOCK_TYPE_COUNT] = {
        //       Luma        |      Chroma
        // DC   AC  4x4  8x8 |    DC      AC
        { 105, 120, 134, 402, 0xFFFF, 0xFFFF }, // Y
        { 484, 499, 513, 660,    149,    152 }, // Cb
        { 528, 543, 557, 718,    149,    152 }  // Cr
    };

    CABACCheck(ctxIdxOffsetForSignificantCoeffFlag[ctxID][type] < CABAC_CONTEXT_MODELS_COUNT);
    return ctxIdxOffsetForSignificantCoeffFlag[ctxID][type];
}

inline uint8_t ContextModelSet::getCtxIdxIncForSignificantCoeffFlag(ChromaArrayType chromaFormat, CodedBlockType type, uint8_t binIdx)
{
    static uint8_t ctxIdxIncForSignificantCoeffFlag8x8[63] = {
         0,  1,  2,  3,  4,  5,  5,  4,
         4,  3,  3,  4,  4,  4,  5,  5,
         4,  4,  4,  4,  3,  3,  6,  7,
         7,  7,  8,  9, 10,  9,  8,  7,
         7,  6, 11, 12, 13, 11,  6,  7,
         8,  9, 14, 10,  9,  8,  6, 11,
        12, 13, 11,  6,  9, 14, 10,  9,
        11, 12, 13, 11, 14, 10, 12
    };

    if (type == CODED_BLOCK_LUMA_8x8)
        return ctxIdxIncForSignificantCoeffFlag8x8[binIdx];
    else if (type == CODED_BLOCK_CHROMA_DC && chromaFormat == CHROMA_FORMAT_422)
        return Min<uint8_t>(binIdx / 2, 2);
    else
        return binIdx;
}

inline uint16_t ContextModelSet::getCtxIdxOffsetForLastSignificantCoeffFlag(ColourComponent ctxID, CodedBlockType type)
{
    static uint16_t ctxIdxOffsetForLastSignificantCoeffFlag[COLOUR_COMPONENT_COUNT][CODED_BLOCK_TYPE_COUNT] = {
        //       Luma        |      Chroma
        // DC   AC  4x4  8x8 |    DC      AC
        { 166, 181, 195, 417, 0xFFFF, 0xFFFF }, // Y
        { 572, 587, 601, 690,    210,    213 }, // Cb
        { 616, 631, 645, 748,    210,    213 }  // Cr
    };

    CABACCheck(ctxIdxOffsetForLastSignificantCoeffFlag[ctxID][type] < CABAC_CONTEXT_MODELS_COUNT);
    return ctxIdxOffsetForLastSignificantCoeffFlag[ctxID][type];
}

inline uint8_t ContextModelSet::getCtxIdxIncForLastSignificantCoeffFlag(ChromaArrayType chromaFormat, CodedBlockType type, uint8_t binIdx)
{
    static uint8_t ctxIdxIncForLastSignificantCoeffFlag8x8[63] = {
        0, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 6, 6, 6, 6,
        7, 7, 7, 7, 8, 8, 8
    };

    if (type == CODED_BLOCK_LUMA_8x8)
        return ctxIdxIncForLastSignificantCoeffFlag8x8[binIdx];
    else if (type == CODED_BLOCK_CHROMA_DC && chromaFormat == CHROMA_FORMAT_422)
        return Min<uint8_t>(binIdx / 2, 2);
    else
        return binIdx;
}

inline uint16_t ContextModelSet::getCtxIdxOffsetForCoeffAbsLevelMinus1(ColourComponent ctxID, CodedBlockType type)
{
    static uint16_t ctxIdxOffsetForCoeffAbsLevelMinus1[COLOUR_COMPONENT_COUNT][CODED_BLOCK_TYPE_COUNT] = {
        //       Luma         |      Chroma
        // DC   AC   4x4  8x8 |    DC      AC
        { 227, 237,  247, 426, 0xFFFF, 0xFFFF }, // Y
        { 952, 962,  972, 708,    257,    266 }, // Cb
        { 982, 992, 1002, 766,    257,    266 }  // Cr
    };

    CABACCheck(ctxIdxOffsetForCoeffAbsLevelMinus1[ctxID][type] < CABAC_CONTEXT_MODELS_COUNT);
    return ctxIdxOffsetForCoeffAbsLevelMinus1[ctxID][type];
}

inline uint8_t ContextModelSet::updateCtxIdxIncForCoeffAbsLevelMinus1(CodedBlockType type, uint8_t ctxIdxInc, uint32_t codeNum)
{
    CABACCheck(ctxIdxInc < ((type == CODED_BLOCK_CHROMA_DC) ? 9 : 10));

    if (ctxIdxInc <= 4)
        ctxIdxInc = (codeNum == 0) ? Min<uint8_t>(ctxIdxInc + 1, 4) : 5 + 1;
    else if (codeNum > 0)
        ctxIdxInc = Min<uint8_t>(ctxIdxInc + 1, (type == CODED_BLOCK_CHROMA_DC) ? 8 : 9);
        
    return ctxIdxInc;
}

#endif
