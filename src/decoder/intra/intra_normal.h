#ifndef __INTRA_NORMAL_H__
#define __INTRA_NORMAL_H__

#define USE_PARTITION_SELECT_SATD false

#include "intra_base.h"

class IntraNormal : public IntraBase {
public:
    IntraNormal(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg);

protected:
    virtual void estimate();
    virtual void decidePartition();
    
    void estimate4x4(ColourComponent compID);
    void estimate8x8(ColourComponent compID);
    void estimate16x16(ColourComponent compID);
    void estimateChroma();
    
    virtual void encodeCoefs4x4(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);
    virtual void encodeCoefs8x8(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);
    virtual void encodeCoefs16x16(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);

    CostType calcModeCost4x4(const Blk<ResType, 4, 4> &resBlk) const;
    CostType calcModeCost4x4(const Blk<ResType, 4, 4> &resBlk, bool sameMode, uint8_t qp) const;
    CostType calcModeCost8x8(const Blk<ResType, 8, 8> &resBlk) const;
    CostType calcModeCost8x8(const Blk<ResType, 8, 8> &resBlk, bool sameMode, uint8_t qp) const;
    template <uint8_t mbW, uint8_t mbH>
    CostType calcModeCostFlatBlock(const Blk<ResType, 4, 4> (&resBlk)[mbH / 4][mbW / 4]) const;

    CostType calcRDCost4x4(
        const Blk<ResType, 4, 4> &rawResBlk, const Blk<ResType, 4, 4> &recResBlk,
        const CoefType (&coefs)[16], uint8_t qp) const;
    CostType calcRDCost8x8(
        const Blk<ResType, 8, 8> &rawResBlk, const Blk<ResType, 8, 8> &recResBlk,
        const CoefType (&coefs)[64], uint8_t qp) const;
    CostType calcRDCost16x16(
        const Blk<ResType, 4, 4> (&rawResBlk)[4][4], const Blk<ResType, 4, 4> (&recResBlk)[4][4],
        const CompCoefs::Blk16x16Type &coefs, uint8_t qp) const;

    CostType cost4x4[COLOUR_COMPONENT_COUNT];
    CostType cost8x8[COLOUR_COMPONENT_COUNT];
    CostType cost16x16[COLOUR_COMPONENT_COUNT];

    Macroblock recMb4x4;
    Macroblock recMb8x8;
    Macroblock recMb16x16;

    CompCoefs coefs4x4[COLOUR_COMPONENT_COUNT];
    CompCoefs coefs8x8[COLOUR_COMPONENT_COUNT];
    CompCoefs coefs16x16[COLOUR_COMPONENT_COUNT];
};

template <uint8_t mbW, uint8_t mbH>
inline CostType IntraNormal::calcModeCostFlatBlock(const Blk<ResType, 4, 4> (&resBlk)[mbH / 4][mbW / 4]) const
{
    CostType cost = 0;

    for (uint8_t yInSbs = 0; yInSbs < mbH / 4; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < mbW / 4; ++xInSbs)
            cost += calcModeCost4x4(resBlk[yInSbs][xInSbs]);
    
    return cost;
}

#endif
