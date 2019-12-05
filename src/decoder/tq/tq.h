#ifndef __TQ_H__
#define __TQ_H__

#include "common/type.h"
#include "common/block.h"
#include "transform.h"
#include "quant.h"

class TQ {
public:
    TQ(ChromaArrayType _chromaFormat, uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset);

    void init(int8_t sliceQPY);
    void init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr);
    void setBiasFactor(double _bias);
    void setScalingMatrices(const Blk<WeightScaleType, 4, 4> &weightScale4x4, const Blk<WeightScaleType, 8, 8> &weightScale8x8);
    uint8_t getMacroblockQP(ColourComponent compID) const { return Q.getMacroblockQP(compID); }
    void updateMacroblockQP(int8_t mbQP);
    void updateMacroblockQP(int8_t mbQPY, int8_t mbQPCb, int8_t mbQPCr);

    void encode4x4(
        ColourComponent compID, const Blk<ResType, 4, 4> &rawRes,
        CoefType coefs[16], Blk<ResType, 4, 4> &recRes) const;
    void encode8x8(
        ColourComponent compID, const Blk<ResType, 8, 8> &rawRes,
        CoefType coefs[64], Blk<ResType, 8, 8> &recRes) const;
    void encode16x16(
        ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[4][4],
        CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][4]) const;
    void encodeChroma(
        ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[2][2],
        CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[2][2]) const;
    void encodeChroma(
        ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[4][2],
        CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][2]) const;

    ///
    void inverse4x4(ColourComponent planeID, CoefType coefs[16], Blk<ResType, 4, 4> &recRes) const;
    void inverse8x8(ColourComponent planeID, CoefType coefs[64], Blk<ResType, 8, 8> &recRes) const;
    void inverse16x16(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][4]) const;
    void inverseChroma(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[2][2]) const;
    void inverseChroma(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][2]) const;

private:
    ChromaArrayType chromaFormat;
    uint8_t bitDepthY;
    uint8_t bitDepthC;
    Quantizer Q;
    InvQuantizer invQ;
};

#endif
