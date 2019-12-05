#ifndef _CODEC_TYPE_H_
#define _CODEC_TYPE_H_

#include "type.h"

const uint32_t NON_ZERO_FLAG_AC_MASK = 0xFFFF;
const uint8_t NON_ZERO_FLAG_DC_SHIFT = 16;

const CostType MAX_COST = ~0U;

// codec
struct PictureLevelConfig {
    ChromaArrayType chromaFormat;
    uint8_t bitDepthY;
    uint8_t bitDepthC;

    uint16_t widthInMbs;
    uint16_t heightInMbs;

    FrameCrop cropInfo;

    int8_t chromaQPIndexOffset;
    int8_t secondChromaQPIndexOffset;

    bool separateColourPlaneFlag;
    bool transform8x8ModeFlag;

    const Blk<WeightScaleType, 4, 4> *scalingMatrix4x4Intra;
    const Blk<WeightScaleType, 8, 8> *scalingMatrix8x8Intra;
    const Blk<WeightScaleType, 4, 4> *scalingMatrix4x4Inter;
    const Blk<WeightScaleType, 8, 8> *scalingMatrix8x8Inter;

    bool entropyCodingModeFlag;

    bool deblockingFilteControlPresentFlag;
};

struct SliceLevelConfig {
    SliceType sliceType;
    uint16_t idrPicId;

    uint32_t firstMbInSlice;
    uint32_t lastMbInSlice;

    // QP
    int8_t sliceQP[COLOUR_COMPONENT_COUNT];

    // CABAC
    uint8_t cabacInitIdc[COLOUR_COMPONENT_COUNT];

    // Deblocking
    uint8_t disableDeblockingFilterIdc[COLOUR_COMPONENT_COUNT];
    int8_t sliceAlphaC0OffsetDiv2[COLOUR_COMPONENT_COUNT];
    int8_t sliceBetaOffsetDiv2[COLOUR_COMPONENT_COUNT];

    double biasFactorIntra;
    double biasFactorInter;
};

struct EncodedMb {
    MacroblockPartition mbPart;
    uint8_t transformSize8x8Flag;

    // Intra
    uint8_t prevIntraPredModeFlag[16];
    uint8_t remIntraPredMode[16];
    uint8_t intra16x16PredMode;
    uint8_t intraChromaPredMode;

    // Common
    CompCoefs coefs;
    uint32_t nonZeroFlags;
    int8_t qp;
    bool qpUpdated;
};

struct MacroblockInfo {
    // Intrinsic
    uint16_t xInMbs;
    uint16_t yInMbs;
    int neighbour;

    // Rate control
    int8_t mbQP[COLOUR_COMPONENT_COUNT];
};

struct MemCtrlToEC {
    CAVLCRefCounts *cavlcRefCounts;
    CABACRefFlags *cabacRefFlags;
};

struct ECToMemCtrl {
    CAVLCCurCounts *cavlcCurCounts;
    CABACCurFlags *cabacCurFlags;
};

struct MemCtrlToDb {
	const Macroblock* curMb;
	const Macroblock* leftMb;
	const Macroblock* upMb;
	DbRefQP* refQP;
};

struct DbToMemCtrl {
	Macroblock* curMb;
	Macroblock* leftMb;
	Macroblock* upMb;
	DbCurQP* curQP;
};

/// for encoder
struct MemCtrlToIntra {
    const Macroblock *orgMb;
    const RefPixels *refPixels;
    const RefIntraModes *refModes;
};

struct IntraToMemCtrl {
    Macroblock *recMb;
    Blk<int, 4, 4> *predModes;
};

////for decoder

struct MemCtrlToRec {
    const RefPixels *refPixels;
    const RefIntraModes *refModes;
};

struct RecToMemCtrl {
    Macroblock *recMb;
    Blk<int, 4, 4> *predModes;
};

#endif