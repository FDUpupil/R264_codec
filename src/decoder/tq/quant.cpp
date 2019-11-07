#include "quant.h"

#define F(a, b) ((double)(a) / (double)(b))

const Blk<double, 4, 4> NormBase4x4 = {
    { F(1, 16), F(1, 20), F(1, 16), F(1, 20) },
    { F(1, 20), F(1, 25), F(1, 20), F(1, 25) },
    { F(1, 16), F(1, 20), F(1, 16), F(1, 20) },
    { F(1, 20), F(1, 25), F(1, 20), F(1, 25) }
};

const Blk<double, 8, 8> NormBase8x8 = {
    { F(1, 64), F(4, 289), F(1, 40), F(4, 289), F(1, 64), F(4, 289), F(1, 40), F(4, 289) },
    { F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521), F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521) },
    { F(1, 40), F(32, 1445), F(1, 25), F(32, 1445), F(1, 40), F(32, 1445), F(1, 25), F(32, 1445) },
    { F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521), F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521) },
    { F(1, 64), F(4, 289), F(1, 40), F(4, 289), F(1, 64), F(4, 289), F(1, 40), F(4, 289) },
    { F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521), F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521) },
    { F(1, 40), F(32, 1445), F(1, 25), F(32, 1445), F(1, 40), F(32, 1445), F(1, 25), F(32, 1445) },
    { F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521), F(4, 289), F(1024, 83521), F(32, 1445), F(1024, 83521) }
};

Quantizer::Quantizer(uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset)
    : bitDepthY(_bitDepthY), bitDepthC(_bitDepthC), cbQPIndexOffset(_chromaQPIndexOffset), crQPIndexOffset(_secondChromaQPIndexOffset)
{
    QuantCheck(8 <= bitDepthY && bitDepthY <= 14);
    QuantCheck(8 <= bitDepthC && bitDepthC <= 14);
    QuantCheck(-12 <= cbQPIndexOffset && cbQPIndexOffset <= 12);
    QuantCheck(-12 <= crQPIndexOffset && crQPIndexOffset <= 12);
}

void Quantizer::setBiasFactor(double _bias)
{
    bias = (uint32_t)((1 << QUANTIZER_BIAS_BIT_DEPTH) * _bias + 0.5);
}

void Quantizer::init(int8_t sliceQPY)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    qp[COLOUR_COMPONENT_Y] = (int8_t)(sliceQPY - qpBdOffsetY);
}

void Quantizer::init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));
    qp[COLOUR_COMPONENT_Y]  = (int8_t)(sliceQPY  - qpBdOffsetY);
    qp[COLOUR_COMPONENT_CB] = (int8_t)(sliceQPCb - qpBdOffsetC);
    qp[COLOUR_COMPONENT_CR] = (int8_t)(sliceQPCr - qpBdOffsetC);
}

void Quantizer::setScalingMatrices(const Blk<WeightScaleType, 4, 4> &weightScale4x4, const Blk<WeightScaleType, 8, 8> &weightScale8x8)
{
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x) {
            double normBase = NormBase4x4[y][x] * (1 << (QUANTIZER_4x4_PRESHIFT_BITS + INV_QUANTIZER_4x4_SCALE_SHIFT_BITS)) * 16 / weightScale4x4[y][x];
            for (int i = 0; i < 6; ++i)
                levelScale4x4[i][y][x] = (uint16_t)(normBase / NormAdjust4x4[i][y][x] + 0.5);
        }

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            double normBase = NormBase8x8[y][x] * (1 << (QUANTIZER_8x8_PRESHIFT_BITS + INV_QUANTIZER_8x8_SCALE_SHIFT_BITS)) * 16 / weightScale8x8[y][x];
            for (int i = 0; i < 6; ++i)
                levelScale8x8[i][y][x] = (uint16_t)(normBase / NormAdjust8x8[i][y][x] + 0.5);
        }
}

void Quantizer::updateMacroblockQP(int8_t mbQP)
{
    uint8_t qpBdOffsetY = (uint8_t)(6 * (bitDepthY - 8));
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));
    int8_t qpIndex;

    QuantCheck(-qpBdOffsetY <= mbQP && mbQP < 52);

    qp[COLOUR_COMPONENT_Y] = mbQP;
    qpScaled[COLOUR_COMPONENT_Y] = (uint8_t)(mbQP + qpBdOffsetY);

    qpIndex = Clip3<int8_t>(-qpBdOffsetC, 51, qp[COLOUR_COMPONENT_Y] + cbQPIndexOffset);
    qp[COLOUR_COMPONENT_CB] = CalcQPChroma(qpIndex);
    qpScaled[COLOUR_COMPONENT_CB] = (uint8_t)(qp[COLOUR_COMPONENT_CB] + qpBdOffsetC);

    qpIndex = Clip3<int8_t>(-qpBdOffsetC, 51, qp[COLOUR_COMPONENT_Y] + crQPIndexOffset);
    qp[COLOUR_COMPONENT_CR] = CalcQPChroma(qpIndex);
    qpScaled[COLOUR_COMPONENT_CR] = (uint8_t)(qp[COLOUR_COMPONENT_CR] + qpBdOffsetC);
}

void Quantizer::updateMacroblockQP(int8_t mbQPY, int8_t mbQPCb, int8_t mbQPCr)
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

void Quantizer::compute2x2DC(ColourComponent compID, const Blk<CoefType, 2, 2> &src, Blk<CoefType, 2, 2> &dst) const
{
    uint8_t curQP;

    QuantCheck((compID == COLOUR_COMPONENT_CB) || (compID == COLOUR_COMPONENT_CR));

    curQP = qpScaled[compID];
    compute<true, 2, 2>(bias, QUANTIZER_4x4_PRESHIFT_BITS + 1 + curQP / 6, (const Blk<uint16_t, 2, 2> &)levelScale4x4[curQP % 6], src, dst);
}

void Quantizer::compute2x4DC(ColourComponent compID, const Blk<CoefType, 2, 4> &src, Blk<CoefType, 2, 4> &dst) const
{
    uint8_t curQP;

    QuantCheck((compID == COLOUR_COMPONENT_CB) || (compID == COLOUR_COMPONENT_CR));

    curQP = qpScaled[compID] + 3;
    compute<true, 2, 4>(bias, QUANTIZER_4x4_PRESHIFT_BITS + 1 + curQP / 6, (const Blk<uint16_t, 2, 4> &)levelScale4x4[curQP % 6], src, dst);
}

void Quantizer::compute4x4DC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<true, 4, 4>(bias, QUANTIZER_4x4_PRESHIFT_BITS + 1 + curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void Quantizer::compute4x4AC(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 4, 4>(bias, QUANTIZER_4x4_PRESHIFT_BITS + curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void Quantizer::compute4x4(ColourComponent compID, const Blk<CoefType, 4, 4> &src, Blk<CoefType, 4, 4> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 4, 4>(bias, QUANTIZER_4x4_PRESHIFT_BITS + curQP / 6, levelScale4x4[curQP % 6], src, dst);
}

void Quantizer::compute8x8(ColourComponent compID, const Blk<CoefType, 8, 8> &src, Blk<CoefType, 8, 8> &dst) const
{
    uint8_t curQP;

    curQP = qpScaled[compID];
    compute<false, 8, 8>(bias, QUANTIZER_8x8_PRESHIFT_BITS + curQP / 6, levelScale8x8[curQP % 6], src, dst);
}
