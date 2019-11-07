#ifndef __TYPE_H__
#define __TYPE_H__

#include <cstdint>

#define Unused(expr) ((void)expr)

#define MAX_SAMPLE_SQ_8 65025
#define MAX_SAMPLE_SQ_10 1046529

const uint32_t NON_ZERO_FLAG_AC_MASK = 0xFFFF;
const uint8_t NON_ZERO_FLAG_DC_SHIFT = 16;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;

using PixType = uint16_t;
using ResType = int16_t;
using CoefType = int32_t;
using CostType = uint32_t;
using WeightScaleType = uint8_t;
//distortion for PSNR
using DistType = int64_t;
using PSNRType = double;

template <typename T, int width, int height>
using Blk = T[height][width];

enum ColourComponent {
    COLOUR_COMPONENT_Y  = 0,
    COLOUR_COMPONENT_CB = 1,
    COLOUR_COMPONENT_CR = 2,
    COLOUR_COMPONENT_COUNT
};

enum ChromaArrayType {
    CHROMA_FORMAT_400 = 0,
    CHROMA_FORMAT_420 = 1,
    CHROMA_FORMAT_422 = 2,
    CHROMA_FORMAT_444 = 3
};

enum SliceType {
    P_SLICE  = 0,
    B_SLICE  = 1,
    I_SLICE  = 7,
    SP_SLICE = 3,
    SI_SLICE = 4
};

enum MbType {
    // I slice
    I_NxN_IN_I_SLICE         = 0,
    I_16x16_0_0_0_IN_I_SLICE = 1,
    I_16x16_1_0_0_IN_I_SLICE = 2,
    I_16x16_2_0_0_IN_I_SLICE = 3,
    I_16x16_3_0_0_IN_I_SLICE = 4,
    I_16x16_0_1_0_IN_I_SLICE = 5,
    I_16x16_1_1_0_IN_I_SLICE = 6,
    I_16x16_2_1_0_IN_I_SLICE = 7,
    I_16x16_3_1_0_IN_I_SLICE = 8,
    I_16x16_0_2_0_IN_I_SLICE = 9,
    I_16x16_1_2_0_IN_I_SLICE = 10,
    I_16x16_2_2_0_IN_I_SLICE = 11,
    I_16x16_3_2_0_IN_I_SLICE = 12,
    I_16x16_0_0_1_IN_I_SLICE = 13,
    I_16x16_1_0_1_IN_I_SLICE = 14,
    I_16x16_2_0_1_IN_I_SLICE = 15,
    I_16x16_3_0_1_IN_I_SLICE = 16,
    I_16x16_0_1_1_IN_I_SLICE = 17,
    I_16x16_1_1_1_IN_I_SLICE = 18,
    I_16x16_2_1_1_IN_I_SLICE = 19,
    I_16x16_3_1_1_IN_I_SLICE = 20,
    I_16x16_0_2_1_IN_I_SLICE = 21,
    I_16x16_1_2_1_IN_I_SLICE = 22,
    I_16x16_2_2_1_IN_I_SLICE = 23,
    I_16x16_3_2_1_IN_I_SLICE = 24,
    I_PCM_IN_I_SLICE         = 25,

    // P slice
    P_L0_16x16_IN_P_SLICE    = 0,
    P_L0_L0_16x8_IN_P_SLICE  = 1,
    P_L0_L0_8x16_IN_P_SLICE  = 2,
    P_8x8_IN_P_SLICE         = 3,
    P_8x8ref0_IN_P_SLICE     = 4,
    I_NxN_IN_P_SLICE         = 5,
    I_16x16_0_0_0_IN_P_SLICE = 6,
    I_16x16_1_0_0_IN_P_SLICE = 7,
    I_16x16_2_0_0_IN_P_SLICE = 8,
    I_16x16_3_0_0_IN_P_SLICE = 9,
    I_16x16_0_1_0_IN_P_SLICE = 10,
    I_16x16_1_1_0_IN_P_SLICE = 11,
    I_16x16_2_1_0_IN_P_SLICE = 12,
    I_16x16_3_1_0_IN_P_SLICE = 13,
    I_16x16_0_2_0_IN_P_SLICE = 14,
    I_16x16_1_2_0_IN_P_SLICE = 15,
    I_16x16_2_2_0_IN_P_SLICE = 16,
    I_16x16_3_2_0_IN_P_SLICE = 17,
    I_16x16_0_0_1_IN_P_SLICE = 18,
    I_16x16_1_0_1_IN_P_SLICE = 19,
    I_16x16_2_0_1_IN_P_SLICE = 20,
    I_16x16_3_0_1_IN_P_SLICE = 21,
    I_16x16_0_1_1_IN_P_SLICE = 22,
    I_16x16_1_1_1_IN_P_SLICE = 23,
    I_16x16_2_1_1_IN_P_SLICE = 24,
    I_16x16_3_1_1_IN_P_SLICE = 25,
    I_16x16_0_2_1_IN_P_SLICE = 26,
    I_16x16_1_2_1_IN_P_SLICE = 27,
    I_16x16_2_2_1_IN_P_SLICE = 28,
    I_16x16_3_2_1_IN_P_SLICE = 29,
    I_PCM_IN_P_SLICE         = 30
};

enum SubMbType {
    // P slice
    P_L0_8x8 = 0,
    P_L0_8x4 = 1,
    P_L0_4x8 = 2,
    P_L0_4x4 = 3
};

enum MacroblockPartition {
    INTRA_PARTITION_4x4,
    INTRA_PARTITION_8x8,
    INTRA_PARTITION_16x16
};

inline bool isIntra(MacroblockPartition mbPart)
{
    return mbPart <= INTRA_PARTITION_16x16;
}

enum NeighbourState {
    NEIGHBOUR_AVAILABLE_LEFT     = (1 << 0),
    NEIGHBOUR_AVAILABLE_UP_LEFT  = (1 << 1),
    NEIGHBOUR_AVAILABLE_UP       = (1 << 2),
    NEIGHBOUR_AVAILABLE_UP_RIGHT = (1 << 3),
//    NEIGHBOUR_AVAILABLE_RIGHT    = (1 << 4)
};

enum CodedBlockType {
    CODED_BLOCK_LUMA_DC,
    CODED_BLOCK_LUMA_AC,
    CODED_BLOCK_LUMA_4x4,
    CODED_BLOCK_LUMA_8x8,
    CODED_BLOCK_CHROMA_DC,
    CODED_BLOCK_CHROMA_AC,
    CODED_BLOCK_TYPE_COUNT
};

struct FrameCrop {
    bool flag;
    uint16_t leftOffset;
    uint16_t rightOffset;
    uint16_t topOffset;
    uint16_t bottomOffset;
};

struct Macroblock {
    Blk<PixType, 16, 16> comp[COLOUR_COMPONENT_COUNT];

    inline Blk<PixType, 16, 16> & operator [](int planeID)
    {
        return comp[planeID];
    }

    inline const Blk<PixType, 16, 16> & operator [](int planeID) const
    {
        return comp[planeID];
    }
};

struct RefPixels {
    PixType l[16]; ///< Left
    PixType ul;    ///< Up-left
    PixType u[24]; ///< Up
};

struct Coordinate {
    uint16_t x;
    uint16_t y;
};

union CompCoefs {
    CoefType blk4x4[16][16];
    CoefType blk8x8[4][64];
    struct Blk16x16Type {
        CoefType dc[16];
        CoefType ac[16][15];
    } blk16x16;
};

template <int n>
struct CordTermFlag2D {
    uint8_t a[n]; // Left
    uint8_t b[n]; // Up
};

template <>
struct CordTermFlag2D<1> {
    uint8_t a; // Left
    uint8_t b; // Up
    CordTermFlag2D() {}
    CordTermFlag2D(uint8_t _a, uint8_t _b) : a(_a), b(_b) {}
};

struct RefIntraModes {
    int l[4];
    int u[4];
};

struct CABACCurFlags {
    uint8_t mbQPDelta;
    uint8_t nonIntraNxN;
    uint8_t intraChromaPredMode;
    uint8_t transform8x8;
    uint8_t cbpLuma;
    uint8_t cbpChroma;
    uint32_t nonZero;
};

struct CABACRefFlags {
    uint8_t mbQPDelta;
    CordTermFlag2D<1> nonIntraNxN;
    CordTermFlag2D<1> intraChromaPredMode;
    CordTermFlag2D<1> transform8x8;
    CordTermFlag2D<2> cbpLuma;
    CordTermFlag2D<1> cbpChroma;
    CordTermFlag2D<4> nonZero;
    CordTermFlag2D<1> nonZeroDC;
};

using CAVLCCurCounts = Blk<uint8_t, 4, 4>;

struct CAVLCRefCounts {
    uint8_t nA[4];
    uint8_t nB[4];
};

using DbCurQP = int8_t;

struct DbRefQP {
    int8_t left;
    int8_t up;
};

#endif
