#ifndef __ENCODER_QUANT_H__
#define __ENCODER_QUANT_H__

#include "common/quant.h"

#include <cassert>

#define QuantCheck(expr) assert(expr)

const int QUANTIZER_4x4_PRESHIFT_BITS = 15;
const int QUANTIZER_8x8_PRESHIFT_BITS = 16;
const int QUANTIZER_BIAS_BIT_DEPTH = 11;

const double QUANTIZER_FLAT_BIAS = 0.5;
const double QUANTIZER_DEFAULT_I_SLICE_BIAS = 1. / 3;
const double QUANTIZER_DEFAULT_P_SLICE_INTRA_BIAS = 1. / 6;
const double QUANTIZER_DEFAULT_P_SLICE_INTER_BIAS = 1. / 6;
const double QUANTIZER_DEFAULT_B_SLICE_INTRA_BIAS = 1. / 6;
const double QUANTIZER_DEFAULT_B_SLICE_INTER_BIAS = 1. / 6;

class Quantizer {
public:
    Quantizer(uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset);

    void init(int8_t sliceQPY);
    void init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr);
    void setBiasFactor(double _bias);
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
    uint32_t bias;
    int8_t qp[COLOUR_COMPONENT_COUNT];
    uint8_t qpScaled[COLOUR_COMPONENT_COUNT];

    Blk<uint16_t, 4, 4> levelScale4x4[6];
    Blk<uint16_t, 8, 8> levelScale8x8[6];

    template <bool isDCMode, int width, int height>
    static void compute(uint32_t bias, uint8_t rshift, const Blk<uint16_t, width, height> &levelScale, const Blk<CoefType, width, height> &src, Blk<CoefType, width, height> &dst);
};

template <bool isDCMode, int width, int height>
inline void Quantizer::compute(uint32_t bias, uint8_t rshift, const Blk<uint16_t, width, height> &levelScale, const Blk<CoefType, width, height> &src, Blk<CoefType, width, height> &dst)
{
    bias <<= rshift - QUANTIZER_BIAS_BIT_DEPTH;
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uint16_t factor = isDCMode ? levelScale[0][0] : levelScale[y][x];
            CoefType level = (uint16_t)((Abs(src[y][x]) * factor + bias) >> rshift);
            dst[y][x] = (src[y][x] >= 0) ? level : -level;
        }
}

#endif
