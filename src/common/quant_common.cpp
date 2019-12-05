#include "quant_common.h"

int8_t CalcQPChroma(int8_t qpIndex)
{
    static const uint8_t QPChromaMap[52 - 30] = {
        29, 30, 31, 32, 32, 33, 34, 34,
        35, 35, 36, 36, 37, 37, 37, 38,
        38, 38, 39, 39, 39, 39
    };

    InvQuantCheck(qpIndex < 52);

    if (qpIndex < 30)
        return qpIndex;
    else // 30 to 51
        return QPChromaMap[qpIndex - 30];
}

InvQuantizer::InvQuantizer(uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset)
    : bitDepthY(_bitDepthY), bitDepthC(_bitDepthC), cbQPIndexOffset(_chromaQPIndexOffset), crQPIndexOffset(_secondChromaQPIndexOffset)
{
    InvQuantCheck(8 <= bitDepthY && bitDepthY <= 14);
    InvQuantCheck(8 <= bitDepthC && bitDepthC <= 14);
    InvQuantCheck(-12 <= cbQPIndexOffset && cbQPIndexOffset <= 12);
    InvQuantCheck(-12 <= crQPIndexOffset && crQPIndexOffset <= 12);
}

void InvQuantizer::init(int8_t sliceQPY)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    qp[COLOUR_COMPONENT_Y] = (int8_t)(sliceQPY - qpBdOffsetY);
}

void InvQuantizer::init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));
    qp[COLOUR_COMPONENT_Y]  = (int8_t)(sliceQPY  - qpBdOffsetY);
    qp[COLOUR_COMPONENT_CB] = (int8_t)(sliceQPCb - qpBdOffsetC);
    qp[COLOUR_COMPONENT_CR] = (int8_t)(sliceQPCr - qpBdOffsetC);
}

void InvQuantizer::setScalingMatrices(const Blk<WeightScaleType, 4, 4> &weightScale4x4, const Blk<WeightScaleType, 8, 8> &weightScale8x8)
{
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            for (int i = 0; i < 6; ++i)
                levelScale4x4[i][y][x] = weightScale4x4[y][x] * NormAdjust4x4[i][y][x];

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            for (int i = 0; i < 6; ++i)
                levelScale8x8[i][y][x] = weightScale8x8[y][x] * NormAdjust8x8[i][y][x];
}

void InvQuantizer::updateMacroblockQP(int8_t mbQP)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));
    int8_t qpIndex;

    InvQuantCheck(-qpBdOffsetY <= mbQP && mbQP < 52);

    qp[COLOUR_COMPONENT_Y] = mbQP;
    qpScaled[COLOUR_COMPONENT_Y] = (uint8_t)(mbQP + qpBdOffsetY);

    qpIndex = Clip3<int8_t>(-qpBdOffsetC, 51, qp[COLOUR_COMPONENT_Y] + cbQPIndexOffset);
    qp[COLOUR_COMPONENT_CB] = CalcQPChroma(qpIndex);
    qpScaled[COLOUR_COMPONENT_CB] = (uint8_t)(qp[COLOUR_COMPONENT_CB] + qpBdOffsetC);

    qpIndex = Clip3<int8_t>(-qpBdOffsetC, 51, qp[COLOUR_COMPONENT_Y] + crQPIndexOffset);
    qp[COLOUR_COMPONENT_CR] = CalcQPChroma(qpIndex);
    qpScaled[COLOUR_COMPONENT_CR] = (uint8_t)(qp[COLOUR_COMPONENT_CR] + qpBdOffsetC);
}

void InvQuantizer::updateMacroblockQP(int8_t mbQPY, int8_t mbQPCb, int8_t mbQPCr)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));

    InvQuantCheck(-qpBdOffsetY <= mbQPY && mbQPY < 52);
    InvQuantCheck(-qpBdOffsetC <= mbQPCb && mbQPCb < 52);
    InvQuantCheck(-qpBdOffsetC <= mbQPCr && mbQPCr < 52);

    qp[COLOUR_COMPONENT_Y] = mbQPY;
    qpScaled[COLOUR_COMPONENT_Y] = (uint8_t)(mbQPY + qpBdOffsetY);

    qp[COLOUR_COMPONENT_CB] = mbQPCb;
    qpScaled[COLOUR_COMPONENT_CB] = (uint8_t)(mbQPCb + qpBdOffsetC);

    qp[COLOUR_COMPONENT_CR] = mbQPCr;
    qpScaled[COLOUR_COMPONENT_CR] = (uint8_t)(mbQPCr + qpBdOffsetC);
}

void InvQuantizer::compute2x2DC(ColourComponent compID, const Blk<CoefType, 2, 2> &src, Blk<CoefType, 2, 2> &dst) const
{
    uint8_t curQP;

    InvQuantCheck((compID == COLOUR_COMPONENT_CB) || (compID == COLOUR_COMPONENT_CR));

    curQP = qpScaled[compID];
#if 1
    // Because Equation 8-326 has different operation compared to other components, we do not use compute() function here
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x)
            dst[y][x] = ((src[y][x] * levelScale4x4[curQP % 6][0][0]) << (curQP / 6)) >> 5;
#else
    compute<true, 2, 2>(5 - curQP / 6, (const Blk<uint16_t, 2, 2> &)levelScale4x4[curQP % 6], src, dst);
#endif
}

void InvQuantizer::compute2x4DC(ColourComponent compID, const Blk<CoefType, 2, 4> &src, Blk<CoefType, 2, 4> &dst) const
{
    uint8_t curQP;

    InvQuantCheck((compID == COLOUR_COMPONENT_CB) || (compID == COLOUR_COMPONENT_CR));

    curQP = qpScaled[compID] + 3;
    compute<true, 2, 4>(INV_QUANTIZER_4x4_SCALE_SHIFT_BITS - curQP / 6, (const Blk<uint16_t, 2, 4> &)levelScale4x4[curQP % 6], src, dst);
}

void InvQuantizer::compute4x4DC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<true, 4, 4>(INV_QUANTIZER_4x4_SCALE_SHIFT_BITS - curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void InvQuantizer::compute4x4AC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 4, 4>(INV_QUANTIZER_4x4_SCALE_SHIFT_BITS - 2 - curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void InvQuantizer::compute4x4(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 4, 4>(INV_QUANTIZER_4x4_SCALE_SHIFT_BITS - 2 - curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void InvQuantizer::compute8x8(ColourComponent compID, const Blk<CoefType, 8, 8> &src, Blk<CoefType, 8, 8> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 8, 8>(INV_QUANTIZER_8x8_SCALE_SHIFT_BITS - 2 - curQP / 6, levelScale8x8[curQP % 6], src, dst);
}
