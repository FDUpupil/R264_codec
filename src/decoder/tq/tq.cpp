#include "tq.h"

TQ::TQ(ChromaArrayType _chromaFormat, uint8_t _bitDepthY, uint8_t _bitDepthC, int8_t _chromaQPIndexOffset, int8_t _secondChromaQPIndexOffset)
    : chromaFormat(_chromaFormat), bitDepthY(_bitDepthY), bitDepthC(_bitDepthC),
      Q(_bitDepthY, _bitDepthC, _chromaQPIndexOffset, _secondChromaQPIndexOffset),
      invQ(_bitDepthY, _bitDepthC, _chromaQPIndexOffset, _secondChromaQPIndexOffset)
{
}

void TQ::init(int8_t sliceQPY)
{
    Q.init(sliceQPY);
    invQ.init(sliceQPY);
}

void TQ::init(int8_t sliceQPY, int8_t sliceQPCb, int8_t sliceQPCr)
{
    Q.init(sliceQPY, sliceQPCb, sliceQPCr);
    invQ.init(sliceQPY, sliceQPCb, sliceQPCr);
}

void TQ::setBiasFactor(double _bias)
{
    Q.setBiasFactor(_bias);
}

void TQ::setScalingMatrices(const Blk<WeightScaleType, 4, 4> &weightScale4x4, const Blk<WeightScaleType, 8, 8> &weightScale8x8)
{
    Q.setScalingMatrices(weightScale4x4, weightScale8x8);
    invQ.setScalingMatrices(weightScale4x4, weightScale8x8);
}

void TQ::updateMacroblockQP(int8_t mbQP)
{
    Q.updateMacroblockQP(mbQP);
    invQ.updateMacroblockQP(mbQP);
}

void TQ::updateMacroblockQP(int8_t mbQPY, int8_t mbQPCb, int8_t mbQPCr)
{
    Q.updateMacroblockQP(mbQPY, mbQPCb, mbQPCr);
    invQ.updateMacroblockQP(mbQPY, mbQPCb, mbQPCr);
}

void TQ::encode4x4(
    ColourComponent compID, const Blk<ResType, 4, 4> &rawRes,
    CoefType coefs[16], Blk<ResType, 4, 4> &recRes) const
{
    Blk<CoefType, 4, 4> coefBlk;

    DCT4x4(rawRes, coefBlk);
    Q.compute4x4(compID, coefBlk, coefBlk);

    ScanZigZag4x4(coefBlk, coefs);

    invQ.compute4x4(compID, coefBlk, coefBlk);
    IDCT4x4(coefBlk, recRes);
}

void TQ::encode8x8(
    ColourComponent compID, const Blk<ResType, 8, 8> &rawRes,
    CoefType coefs[64], Blk<ResType, 8, 8> &recRes) const
{
    Blk<CoefType, 8, 8> coefBlk;

    DCT8x8(rawRes, coefBlk);
    Q.compute8x8(compID, coefBlk, coefBlk);

    ScanZigZag8x8(coefBlk, coefs);

    invQ.compute8x8(compID, coefBlk, coefBlk);
    IDCT8x8(coefBlk, recRes);
}

void TQ::encode16x16(
    ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[4][4],
    CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][4]) const
{
    Blk<CoefType, 4, 4> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[4][4];

    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;

        DCT4x4(rawRes[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkDC[yInSbs][xInSbs] = coefBlkAC[yInSbs][xInSbs][0][0];
        Q.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);

        ScanZigZag4x4(coefBlkAC[yInSbs][xInSbs], coefs.ac[idx], 1, 15);
    }

    WHT4x4(coefBlkDC, coefBlkDC);
    Q.compute4x4DC(compID, coefBlkDC, coefBlkDC);

    ScanZigZag4x4(coefBlkDC, coefs.dc);

    IWHT4x4(coefBlkDC, coefBlkDC);
    invQ.compute4x4DC(compID, coefBlkDC, coefBlkDC);

    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;

        invQ.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}

void TQ::encodeChroma(
    ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[2][2],
    CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[2][2]) const
{
    Blk<CoefType, 2, 2> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[2][2];

    for (uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;

        DCT4x4(rawRes[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkDC[yInSbs][xInSbs] = coefBlkAC[yInSbs][xInSbs][0][0];
        Q.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);

        ScanZigZag4x4(coefBlkAC[yInSbs][xInSbs], coefs.ac[idx], 1, 15);
    }

    WHT2x2(coefBlkDC, coefBlkDC);
    Q.compute2x2DC(compID, coefBlkDC, coefBlkDC);

    ScanZigZag2x2(coefBlkDC, coefs.dc);

    IWHT2x2(coefBlkDC, coefBlkDC);
    invQ.compute2x2DC(compID, coefBlkDC, coefBlkDC);

    for (uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;

        invQ.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}

void TQ::encodeChroma(
    ColourComponent compID, const Blk<ResType, 4, 4> (&rawRes)[4][2],
    CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][2]) const
{
    Blk<CoefType, 2, 4> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[4][2];

    for (uint8_t idx = 0; idx < 8; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x4[idx].y;

        DCT4x4(rawRes[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkDC[yInSbs][xInSbs] = coefBlkAC[yInSbs][xInSbs][0][0];
        Q.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);

        ScanZigZag4x4(coefBlkAC[yInSbs][xInSbs], coefs.ac[idx], 1, 15);
    }

    WHT2x4(coefBlkDC, coefBlkDC);
    Q.compute2x4DC(compID, coefBlkDC, coefBlkDC);

    ScanZigZag2x4(coefBlkDC, coefs.dc);

    IWHT2x4(coefBlkDC, coefBlkDC);
    invQ.compute2x4DC(compID, coefBlkDC, coefBlkDC);

    for (uint8_t idx = 0; idx < 8; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x4[idx].y;

        invQ.compute4x4AC(compID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}

///
void TQ::inverse4x4(ColourComponent planeID, CoefType coefs[16], Blk<ResType, 4, 4> &recRes) const
{
    Blk<CoefType, 4, 4> coefBlk;

    InvScanZigZag4x4(coefs, coefBlk);

    invQ.compute4x4(planeID, coefBlk, coefBlk);
    IDCT4x4(coefBlk, recRes);
}

void TQ::inverse8x8(ColourComponent planeID, CoefType coefs[64], Blk<ResType, 8, 8> &recRes) const
{
    Blk<CoefType, 8, 8> coefBlk;

    InvScanZigZag8x8(coefs, coefBlk);

    invQ.compute8x8(planeID, coefBlk, coefBlk);
    IDCT8x8(coefBlk, recRes);
}

void TQ::inverse16x16(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][4]) const
{
    Blk<CoefType, 4, 4> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[4][4];

    InvScanZigZag4x4(coefs.dc, coefBlkDC);

    IWHT4x4(coefBlkDC, coefBlkDC);
    invQ.compute4x4DC(planeID, coefBlkDC, coefBlkDC);

    for(uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;

        InvScanZigZag4x4(coefs.ac[idx], coefBlkAC[yInSbs][xInSbs], 1, 15);
        
        invQ.compute4x4AC(planeID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}

void TQ::inverseChroma(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[2][2]) const
{
    Blk<CoefType, 2, 2> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[2][2];

    InvScanZigZag2x2(coefs.dc, coefBlkDC);
	IWHT2x2(coefBlkDC, coefBlkDC);
	invQ.compute2x2DC(planeID, coefBlkDC, coefBlkDC);

    for(uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;

        InvScanZigZag4x4(coefs.ac[idx], coefBlkAC[yInSbs][xInSbs], 1, 15);

        invQ.compute4x4AC(planeID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}

void TQ::inverseChroma(ColourComponent planeID, CompCoefs::Blk16x16Type &coefs, Blk<ResType, 4, 4> (&recRes)[4][2]) const
{
    Blk<CoefType, 2, 4> coefBlkDC;
    Blk<CoefType, 4, 4> coefBlkAC[4][2];

    InvScanZigZag2x4(coefs.dc, coefBlkDC);
	IWHT2x4(coefBlkDC, coefBlkDC);
	invQ.compute2x4DC(planeID, coefBlkDC, coefBlkDC);

    for(uint8_t idx = 0; idx < 8; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x4[idx].y;

        InvScanZigZag4x4(coefs.ac[idx], coefBlkAC[yInSbs][xInSbs], 1, 15);
        
        invQ.compute4x4AC(planeID, coefBlkAC[yInSbs][xInSbs], coefBlkAC[yInSbs][xInSbs]);
        coefBlkAC[yInSbs][xInSbs][0][0] = coefBlkDC[yInSbs][xInSbs];
        IDCT4x4(coefBlkAC[yInSbs][xInSbs], recRes[yInSbs][xInSbs]);
    }
}
