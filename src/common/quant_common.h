#ifndef __QUANT_H__
#define __QUANT_H__

#include "type.h"
#include "math.h"

#include <cassert>

#define InvQuantCheck(expr) assert(expr)

const int INV_QUANTIZER_4x4_SCALE_SHIFT_BITS = 6;
const int INV_QUANTIZER_8x8_SCALE_SHIFT_BITS = 8;

const Blk<WeightScaleType, 4, 4> ScalingMatrixFlat4x4 = {
    { 16, 16, 16, 16 },
    { 16, 16, 16, 16 },
    { 16, 16, 16, 16 },
    { 16, 16, 16, 16 }
};

const Blk<WeightScaleType, 8, 8> ScalingMatrixFlat8x8 = {
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 },
    { 16, 16, 16, 16, 16, 16, 16, 16 }
};

const Blk<WeightScaleType, 4, 4> ScalingMatrixDefault4x4Intra = {
    {  6, 13, 20, 28 },
    { 13, 20, 28, 32 },
    { 20, 28, 32, 37 },
    { 28, 32, 37, 42 }
};

const Blk<WeightScaleType, 4, 4> ScalingMatrixDefault4x4Inter = {
    { 10, 14, 20, 24 },
    { 14, 20, 24, 27 },
    { 20, 24, 27, 30 },
    { 24, 27, 30, 34 }
};

const Blk<WeightScaleType, 8, 8> ScalingMatrixDefault8x8Intra = {
    {  6, 10, 13, 16, 18, 23, 25, 27 },
    { 10, 11, 16, 18, 23, 25, 27, 29 },
    { 13, 16, 18, 23, 25, 27, 29, 31 },
    { 16, 18, 23, 25, 27, 29, 31, 33 },
    { 18, 23, 25, 27, 29, 31, 33, 36 },
    { 23, 25, 27, 29, 31, 33, 36, 38 },
    { 25, 27, 29, 31, 33, 36, 38, 40 },
    { 27, 29, 31, 33, 36, 38, 40, 42 }
};

const Blk<WeightScaleType, 8, 8> ScalingMatrixDefault8x8Inter = {
    {  9, 13, 15, 17, 19, 21, 22, 24 },
    { 13, 13, 17, 19, 21, 22, 24, 25 },
    { 15, 17, 19, 21, 22, 24, 25, 27 },
    { 17, 19, 21, 22, 24, 25, 27, 28 },
    { 19, 21, 22, 24, 25, 27, 28, 30 },
    { 21, 22, 24, 25, 27, 28, 30, 32 },
    { 22, 24, 25, 27, 28, 30, 32, 33 },
    { 24, 25, 27, 28, 30, 32, 33, 35 }
};

const Blk<uint8_t, 4, 4> NormAdjust4x4[6] = {
    {
        { 10, 13, 10, 13 },
        { 13, 16, 13, 16 },
        { 10, 13, 10, 13 },
        { 13, 16, 13, 16 }
    }, {
        { 11, 14, 11, 14 },
        { 14, 18, 14, 18 },
        { 11, 14, 11, 14 },
        { 14, 18, 14, 18 }
    }, {
        { 13, 16, 13, 16 },
        { 16, 20, 16, 20 },
        { 13, 16, 13, 16 },
        { 16, 20, 16, 20 }
    }, {
        { 14, 18, 14, 18 },
        { 18, 23, 18, 23 },
        { 14, 18, 14, 18 },
        { 18, 23, 18, 23 }
    }, {
        { 16, 20, 16, 20 },
        { 20, 25, 20, 25 },
        { 16, 20, 16, 20 },
        { 20, 25, 20, 25 }
    }, {
        { 18, 23, 18, 23 },
        { 23, 29, 23, 29 },
        { 18, 23, 18, 23 },
        { 23, 29, 23, 29 }
    }
};

const Blk<uint8_t, 8, 8> NormAdjust8x8[6] = {
    {
        { 20, 19, 25, 19, 20, 19, 25, 19 },
        { 19, 18, 24, 18, 19, 18, 24, 18 },
        { 25, 24, 32, 24, 25, 24, 32, 24 },
        { 19, 18, 24, 18, 19, 18, 24, 18 },
        { 20, 19, 25, 19, 20, 19, 25, 19 },
        { 19, 18, 24, 18, 19, 18, 24, 18 },
        { 25, 24, 32, 24, 25, 24, 32, 24 },
        { 19, 18, 24, 18, 19, 18, 24, 18 }
    }, {
        { 22, 21, 28, 21, 22, 21, 28, 21 },
        { 21, 19, 26, 19, 21, 19, 26, 19 },
        { 28, 26, 35, 26, 28, 26, 35, 26 },
        { 21, 19, 26, 19, 21, 19, 26, 19 },
        { 22, 21, 28, 21, 22, 21, 28, 21 },
        { 21, 19, 26, 19, 21, 19, 26, 19 },
        { 28, 26, 35, 26, 28, 26, 35, 26 },
        { 21, 19, 26, 19, 21, 19, 26, 19 }
    }, {
        { 26, 24, 33, 24, 26, 24, 33, 24 },
        { 24, 23, 31, 23, 24, 23, 31, 23 },
        { 33, 31, 42, 31, 33, 31, 42, 31 },
        { 24, 23, 31, 23, 24, 23, 31, 23 },
        { 26, 24, 33, 24, 26, 24, 33, 24 },
        { 24, 23, 31, 23, 24, 23, 31, 23 },
        { 33, 31, 42, 31, 33, 31, 42, 31 },
        { 24, 23, 31, 23, 24, 23, 31, 23 }
    }, {
        { 28, 26, 35, 26, 28, 26, 35, 26 },
        { 26, 25, 33, 25, 26, 25, 33, 25 },
        { 35, 33, 45, 33, 35, 33, 45, 33 },
        { 26, 25, 33, 25, 26, 25, 33, 25 },
        { 28, 26, 35, 26, 28, 26, 35, 26 },
        { 26, 25, 33, 25, 26, 25, 33, 25 },
        { 35, 33, 45, 33, 35, 33, 45, 33 },
        { 26, 25, 33, 25, 26, 25, 33, 25 }
    }, {
        { 32, 30, 40, 30, 32, 30, 40, 30 },
        { 30, 28, 38, 28, 30, 28, 38, 28 },
        { 40, 38, 51, 38, 40, 38, 51, 38 },
        { 30, 28, 38, 28, 30, 28, 38, 28 },
        { 32, 30, 40, 30, 32, 30, 40, 30 },
        { 30, 28, 38, 28, 30, 28, 38, 28 },
        { 40, 38, 51, 38, 40, 38, 51, 38 },
        { 30, 28, 38, 28, 30, 28, 38, 28 }
    }, {
        { 36, 34, 46, 34, 36, 34, 46, 34 },
        { 34, 32, 43, 32, 34, 32, 43, 32 },
        { 46, 43, 58, 43, 46, 43, 58, 43 },
        { 34, 32, 43, 32, 34, 32, 43, 32 },
        { 36, 34, 46, 34, 36, 34, 46, 34 },
        { 34, 32, 43, 32, 34, 32, 43, 32 },
        { 46, 43, 58, 43, 46, 43, 58, 43 },
        { 34, 32, 43, 32, 34, 32, 43, 32 }
    }
};

int8_t CalcQPChroma(int8_t qpIndex);

class InvQuantizer {
public:
    InvQuantizer(uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset);
    
    void init(int8_t sliceQPY);
    void init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr);
    void setScalingMatrices(const Blk<WeightScaleType, 4, 4> &weightScale4x4, const Blk<WeightScaleType, 8, 8> &weightScale8x8);
    uint8_t getMacroblockQP(ColourComponent compID) const { return qpScaled[compID]; }
    void updateMacroblockQP(int8_t mbQP);
    void updateMacroblockQP(int8_t mbQPY, int8_t mbQPCb, int8_t mbQPCr);

    void compute2x2DC(ColourComponent compID, const Blk<CoefType, 2, 2> &src, Blk<CoefType, 2, 2> &dst) const;
    void compute2x4DC(ColourComponent compID, const Blk<CoefType, 2, 4> &src, Blk<CoefType, 2, 4> &dst) const;    
    void compute4x4DC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const;
    void compute4x4AC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const;
    void compute4x4(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const;
    void compute8x8(ColourComponent compID, const Blk<CoefType, 8, 8> &src, Blk<CoefType, 8, 8> &dst) const;

protected:
    uint8_t bitDepthY;
    uint8_t bitDepthC;
    int8_t cbQPIndexOffset;
    int8_t crQPIndexOffset;
    int8_t qp[COLOUR_COMPONENT_COUNT];
    uint8_t qpScaled[COLOUR_COMPONENT_COUNT];

    Blk<uint16_t, 4, 4> levelScale4x4[6];
    Blk<uint16_t, 8, 8> levelScale8x8[6];

    template <bool isDCMode, int width, int height>
    static void compute(int8_t rshift, const Blk<uint16_t, width, height> &levelScale, const Blk<CoefType, width, height> &src, Blk<CoefType, width, height> &dst);
};

template <bool isDCMode, int width, int height>
inline void InvQuantizer::compute(int8_t rshift, const Blk<uint16_t, width, height> &levelScale, const Blk<CoefType, width, height> &src, Blk<CoefType, width, height> &dst)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uint16_t factor = isDCMode ? levelScale[0][0] : levelScale[y][x];
            if (rshift > 0)
                dst[y][x] = (CoefType)RoundShift<int32_t>(src[y][x] * factor, (uint8_t)rshift);
            else
                dst[y][x] = (src[y][x] * factor) << -rshift;
        }
}

#endif
