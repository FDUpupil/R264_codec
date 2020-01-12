#include "deblock.h"
#include "common/math.h"
#include "common/quant_common.h"

#define PIXELS_HOR(blk, xOff, yOff) \
    blk[yOff][xOff + 0], \
    blk[yOff][xOff + 1], \
    blk[yOff][xOff + 2], \
    blk[yOff][xOff + 3]

#define PIXELS_VER(blk, xOff, yOff) \
    blk[yOff + 0][xOff], \
    blk[yOff + 1][xOff], \
    blk[yOff + 2][xOff], \
    blk[yOff + 3][xOff]

const uint8_t DeblockingFilter::alphaMap[52] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      4,   4,   5,   6,   7,   8,   9,  10,
     12,  13,  15,  17,  20,  22,  25,  28,
     32,  36,  40,  45,  50,  56,  63,  71,
     80,  90, 101, 113, 127, 144, 162, 182,
    203, 226, 255, 255
};

const uint8_t DeblockingFilter::betaMap[52] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,
     2,  2,  2,  3,  3,  3,  3,  4,
     4,  4,  6,  6,  7,  7,  8,  8,
     9,  9, 10, 10, 11, 11, 12, 12,
    13, 13, 14, 14, 15, 15, 16, 16,
    17, 17, 18, 18
};

const uint8_t DeblockingFilter::tc0Map[3][52] = {
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  1,
         1,  1,  1,  1,  1,  1,  1,  1,
         1,  2,  2,  2,  2,  3,  3,  3,
         4,  4,  4,  5,  6,  6,  7,  8,
         9, 10, 11, 13
    }, {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  2,
         2,  2,  2,  3,  3,  3,  4,  4,
         5,  5,  6,  7,  8,  8, 10, 11,
        12, 13, 15, 17
    }, {
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  2,  2,  2,  2,  3,
         3,  3,  4,  4,  4,  5,  6,  6,
         7,  8,  9, 10, 11, 13, 14, 16,
        18, 20, 23, 25
    }
};

DeblockingFilter::DeblockingFilter(const PictureLevelConfig &cfg)
    : chromaFormat(cfg.chromaFormat),
      bitDepthY(cfg.bitDepthY), bitDepthC(cfg.bitDepthC),
      cbQPIndexOffset(cfg.chromaQPIndexOffset), crQPIndexOffset(cfg.secondChromaQPIndexOffset),
      separateColourPlaneFlag(cfg.separateColourPlaneFlag)
{
    compCount = (uint8_t)(separateColourPlaneFlag ? COLOUR_COMPONENT_COUNT : 1);
    planeCount = (uint8_t)((chromaFormat != CHROMA_FORMAT_400) ? COLOUR_COMPONENT_COUNT : 1);
}

void DeblockingFilter::init(const SliceLevelConfig &cfg)
{
    for (int compID = 0; compID < compCount; ++compID) {
        disableDeblockingFilterIdc[compID] = cfg.disableDeblockingFilterIdc[compID];
        filterOffsetA[compID] = cfg.sliceAlphaC0OffsetDiv2[compID] << 1;
        filterOffsetB[compID] = cfg.sliceBetaOffsetDiv2[compID] << 1;
    }
}

void DeblockingFilter::cycle(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut)
{
    start(mbInfo, mbEnc, memIn, memOut);

    preprocess();
    for (int compID = 0; compID < compCount; ++compID)
        deriveStrength((ColourComponent)compID);
    filterLumaBlk(COLOUR_COMPONENT_Y);
    if (chromaFormat == CHROMA_FORMAT_444) {
        filterLumaBlk(COLOUR_COMPONENT_CB);
        filterLumaBlk(COLOUR_COMPONENT_CR);
    } else if (chromaFormat == CHROMA_FORMAT_420 || chromaFormat == CHROMA_FORMAT_422) {
        filterChromaBlk(COLOUR_COMPONENT_CB);
        filterChromaBlk(COLOUR_COMPONENT_CR);
    }
    postprocess();

    finish(mbInfo, mbEnc, memIn, memOut);
}

void DeblockingFilter::start(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut)
{
    // MacroblockInfo
    xInMbs = mbInfo.xInMbs;
    yInMbs = mbInfo.yInMbs;
    neighbour = mbInfo.neighbour;

    // EncodedMb
    mb = mbEnc;

    // MemCtrlToDb
    // This module modifies samples in place, so input macroblock is not used
    refQP = memIn.refQP;

    // DbToMemCtrl
    curMb = memOut.curMb;
    leftMb = memOut.leftMb;
    upMb = memOut.upMb;
    curQP = memOut.curQP;
}

void DeblockingFilter::finish(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut)
{
    Unused(mbInfo);
    Unused(mbEnc);
    Unused(memIn);
    Unused(memOut);
}

void DeblockingFilter::preprocess()
{
    setQP();
}

void DeblockingFilter::postprocess()
{
    // Simply do nothing
}

void DeblockingFilter::setQP()
{
    uint8_t qpBdOffsetC = (uint8_t)(6 * (bitDepthC - 8));

    curQP[COLOUR_COMPONENT_Y] = mb[COLOUR_COMPONENT_Y].qp;
    if (separateColourPlaneFlag) {
        curQP[COLOUR_COMPONENT_CB] = mb[COLOUR_COMPONENT_CB].qp;
        curQP[COLOUR_COMPONENT_CR] = mb[COLOUR_COMPONENT_CR].qp;
    } else if (chromaFormat != CHROMA_FORMAT_400) {
        curQP[COLOUR_COMPONENT_CB] = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, curQP[COLOUR_COMPONENT_Y] + cbQPIndexOffset));
        curQP[COLOUR_COMPONENT_CR] = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, curQP[COLOUR_COMPONENT_Y] + crQPIndexOffset));
    }

    if (!separateColourPlaneFlag && chromaFormat != CHROMA_FORMAT_400) {
        refQP[COLOUR_COMPONENT_CB].left = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, refQP[COLOUR_COMPONENT_Y].left + cbQPIndexOffset));
        refQP[COLOUR_COMPONENT_CR].left = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, refQP[COLOUR_COMPONENT_Y].left + crQPIndexOffset));
        refQP[COLOUR_COMPONENT_CB].up = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, refQP[COLOUR_COMPONENT_Y].up + cbQPIndexOffset));
        refQP[COLOUR_COMPONENT_CR].up = CalcQPChroma(Clip3<int8_t>(-qpBdOffsetC, 51, refQP[COLOUR_COMPONENT_Y].up + crQPIndexOffset));
    }
}

void DeblockingFilter::deriveStrength(ColourComponent compID)
{
    // Vertical
    for (int xInSbs = 0; xInSbs < 4; ++xInSbs)
        for (int yInSbs = 0; yInSbs < 4; ++yInSbs)
            if (true) {
                // Intra
                if (xInSbs == 0)
                    bSVer[compID][xInSbs][yInSbs] = 4;
                else
                    bSVer[compID][xInSbs][yInSbs] = 3;
            }
    
    // Horizontal
    for (int yInSbs = 0; yInSbs < 4; ++yInSbs)
        for (int xInSbs = 0; xInSbs < 4; ++xInSbs)
            if (true) {
                // Intra
                if (yInSbs == 0)
                    bSHor[compID][yInSbs][xInSbs] = 4;
                else
                    bSHor[compID][yInSbs][xInSbs] = 3;
            }
}

void DeblockingFilter::filterLumaBlk(ColourComponent planeID)
{
    ColourComponent compID = separateColourPlaneFlag ? planeID : COLOUR_COMPONENT_Y;
    uint8_t indexA = (uint8_t)Clip3<int8_t>(0, 51, curQP[planeID] + filterOffsetA[compID]);
    uint8_t indexB = (uint8_t)Clip3<int8_t>(0, 51, curQP[planeID] + filterOffsetB[compID]);

    if (disableDeblockingFilterIdc[compID] == 1)
        return;

    // Vertical

    // (0, k)
    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT || (disableDeblockingFilterIdc[compID] == 0 && xInMbs != 0)) {
        int8_t qpAvg = (refQP[planeID].left + curQP[planeID] + 1) >> 1;
        uint8_t indexALeft = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetA[compID]);
        uint8_t indexBLeft = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetB[compID]);
        for (uint8_t yOfMbs = 0; yOfMbs < 16; ++yOfMbs)
            filterLumaEdge(
                planeID, indexALeft, indexBLeft, bSVer[compID][0][yOfMbs / 4],
                PIXELS_HOR((*leftMb)[planeID], 12, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 0, yOfMbs));
    }

    // (4, k)
    if (!mb[compID].transformSize8x8Flag) {
        for (uint8_t yOfMbs = 0; yOfMbs < 16; ++yOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSVer[compID][1][yOfMbs / 4],
                PIXELS_HOR((*curMb)[planeID], 0, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 4, yOfMbs));
    }

    // (8, k)
    {
        for (uint8_t yOfMbs = 0; yOfMbs < 16; ++yOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSVer[compID][2][yOfMbs / 4],
                PIXELS_HOR((*curMb)[planeID], 4, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 8, yOfMbs));
    }

    // (12, k)
    if (!mb[compID].transformSize8x8Flag) {
        for (uint8_t yOfMbs = 0; yOfMbs < 16; ++yOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSVer[compID][3][yOfMbs / 4],
                PIXELS_HOR((*curMb)[planeID], 8, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 12, yOfMbs));
    }

    // Horizontal

    // (k, 0)
    if (neighbour & NEIGHBOUR_AVAILABLE_UP || (disableDeblockingFilterIdc[compID] == 0 && yInMbs != 0)) {
        int8_t qpAvg = (refQP[planeID].up + curQP[planeID] + 1) >> 1;
        uint8_t indexAUp = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetA[compID]);
        uint8_t indexBUp = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetB[compID]);
        for (uint8_t xOfMbs = 0; xOfMbs < 16; ++xOfMbs)
            filterLumaEdge(
                planeID, indexAUp, indexBUp, bSHor[compID][0][xOfMbs / 4],
                PIXELS_VER((*upMb)[planeID], xOfMbs, 12),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 0));
    }

    // (k, 4)
    if (!mb[compID].transformSize8x8Flag) {
        for (uint8_t xOfMbs = 0; xOfMbs < 16; ++xOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSHor[compID][1][xOfMbs / 4],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 0),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 4));
    }

    // (k, 8)
    {
        for (uint8_t xOfMbs = 0; xOfMbs < 16; ++xOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSHor[compID][2][xOfMbs / 4],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 4),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 8));
    }

    // (k, 12)
    if (!mb[compID].transformSize8x8Flag) {
        for (uint8_t xOfMbs = 0; xOfMbs < 16; ++xOfMbs)
            filterLumaEdge(
                planeID, indexA, indexB, bSHor[compID][3][xOfMbs / 4],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 8),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 12));
    }
}

void DeblockingFilter::filterChromaBlk(ColourComponent planeID)
{
    uint8_t mbW = 8;
    uint8_t mbH = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;
    uint8_t indexA = (uint8_t)Clip3<int8_t>(0, 51, curQP[planeID] + filterOffsetA[COLOUR_COMPONENT_Y]);
    uint8_t indexB = (uint8_t)Clip3<int8_t>(0, 51, curQP[planeID] + filterOffsetB[COLOUR_COMPONENT_Y]);

    if (disableDeblockingFilterIdc[COLOUR_COMPONENT_Y] == 1)
        return;

    // Vertical

    // (0, k)
    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT || (disableDeblockingFilterIdc[COLOUR_COMPONENT_Y] == 0 && xInMbs != 0)) {
        int8_t qpAvg = (refQP[planeID].left + curQP[planeID] + 1) >> 1;
        uint8_t indexALeft = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetA[COLOUR_COMPONENT_Y]);
        uint8_t indexBLeft = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetB[COLOUR_COMPONENT_Y]);
        for (uint8_t yOfMbs = 0; yOfMbs < mbH; ++yOfMbs)
            filterChromaEdge(
                indexALeft, indexBLeft, bSVer[COLOUR_COMPONENT_Y][0][yOfMbs / (mbH / 4)],
                PIXELS_HOR((*leftMb)[planeID], mbW - 4, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 0, yOfMbs));
    }

    // (4, k)
    {
        for (uint8_t yOfMbs = 0; yOfMbs < mbH; ++yOfMbs)
            filterChromaEdge(
                indexA, indexB, bSVer[COLOUR_COMPONENT_Y][1][yOfMbs / (mbH / 4)],
                PIXELS_HOR((*curMb)[planeID], 0, yOfMbs),
                PIXELS_HOR((*curMb)[planeID], 4, yOfMbs));
    }

    // Horizontal

    // (k, 0)
    if (neighbour & NEIGHBOUR_AVAILABLE_UP || (disableDeblockingFilterIdc[COLOUR_COMPONENT_Y] == 0 && yInMbs != 0)) {
        int8_t qpAvg = (refQP[planeID].up + curQP[planeID] + 1) >> 1;
        uint8_t indexAUp = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetA[COLOUR_COMPONENT_Y]);
        uint8_t indexBUp = (uint8_t)Clip3<int8_t>(0, 51, qpAvg + filterOffsetB[COLOUR_COMPONENT_Y]);
        for (uint8_t xOfMbs = 0; xOfMbs < mbW; ++xOfMbs)
            filterChromaEdge(
                indexAUp, indexBUp, bSHor[COLOUR_COMPONENT_Y][0][xOfMbs / (mbW / 4)],
                PIXELS_VER((*upMb)[planeID], xOfMbs, mbH - 4),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 0));
    }

    // (k, 4)
    {
        for (uint8_t xOfMbs = 0; xOfMbs < mbW; ++xOfMbs)
            filterChromaEdge(
                indexA, indexB, bSHor[COLOUR_COMPONENT_Y][1][xOfMbs / (mbW / 4)],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 0),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 4));
    }

    // (k, 8)
    if (mbH == 16) {
        for (uint8_t xOfMbs = 0; xOfMbs < mbW; ++xOfMbs)
            filterChromaEdge(
                indexA, indexB, bSHor[COLOUR_COMPONENT_Y][2][xOfMbs / (mbW / 4)],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 4),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 8));
    }

    // (k, 12)
    if (mbH == 16) {
        for (uint8_t xOfMbs = 0; xOfMbs < mbW; ++xOfMbs)
            filterChromaEdge(
                indexA, indexB, bSHor[COLOUR_COMPONENT_Y][3][xOfMbs / (mbW / 4)],
                PIXELS_VER((*curMb)[planeID], xOfMbs, 8),
                PIXELS_VER((*curMb)[planeID], xOfMbs, 12));
    }
}

void DeblockingFilter::filterLumaEdge(
        ColourComponent planeID, uint8_t indexA, uint8_t indexB, uint8_t bS,
        PixType &p3, PixType &p2, PixType &p1, PixType &p0,
        PixType &q0, PixType &q1, PixType &q2, PixType &q3) const
{
    uint8_t bitDepth = (planeID == COLOUR_COMPONENT_Y) ? bitDepthY : bitDepthC;
    PixType alpha = alphaMap[indexA] << (bitDepth - 8);
    PixType beta = betaMap[indexB] << (bitDepth - 8);
    bool filterSamplesFlag = bS != 0 && Abs<ResType>(p0 - q0) < alpha &&
        Abs<ResType>(p1 - p0) < beta && Abs<ResType>(q1 - q0) < beta;
    PixType ap = (PixType)Abs<ResType>(p2 - p0);
    PixType aq = (PixType)Abs<ResType>(q2 - q0);
    
    if (!filterSamplesFlag) {
        return;
    } else if (bS < 4) {
        PixType tc0 = tc0Map[bS - 1][indexA] << (bitDepth - 8);
        PixType tc = tc0 + (ap < beta ? 1 : 0) + (aq < beta ? 1 : 0);
        ResType delta = Clip3<ResType>(-tc, tc, (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3);
        PixType p0n = (PixType)Clip1<ResType>(p0 + delta, bitDepth);
        PixType q0n = (PixType)Clip1<ResType>(q0 - delta, bitDepth);
        PixType p1n;
        PixType q1n;

        if (ap < beta)
            p1n = (PixType)(p1 + Clip3<ResType>(-tc0, tc0, (p2 + ((p0 + q0 + 1) >> 1) - (p1 << 1)) >> 1));
        else
            p1n = p1;
        if (aq < beta)
            q1n = (PixType)(q1 + Clip3<ResType>(-tc0, tc0, (q2 + ((p0 + q0 + 1) >> 1) - (q1 << 1)) >> 1));
        else
            q1n = q1;

        p0 = p0n;
        if (ap < beta)
            p1 = p1n;
        q0 = q0n;
        if (aq < beta)
            q1 = q1n;
    } else {
        bool flag = Abs<ResType>(p0 - q0) < (alpha >> 2) + 2;
        bool strongA = ap < beta && flag;
        bool strongB = aq < beta && flag;
        PixType p0n;
        PixType p1n;
        PixType p2n;
        PixType q0n;
        PixType q1n;
        PixType q2n;

        if (strongA) {
            p0n = (p2 + 2 * p1 + 2 * p0 + 2 * q0 + q1 + 4) >> 3;
            p1n = (p2 + p1 + p0 + q0 + 2) >> 2;
            p2n = (2 * p3 + 3 * p2 + p1 + p0 + q0 + 4) >> 3;
        } else {
            p0n = (2 * p1 + p0 + q1 + 2) >> 2;
            p1n = p1;
            p2n = p2;
        }

        if (strongB) {
            q0n = (p1 + 2 * p0 + 2 * q0 + 2 * q1 + q2 + 4) >> 3;
            q1n = (p0 + q0 + q1 + q2 + 2) >> 2;
            q2n = (2 * q3 + 3 * q2 + q1 + q0 + p0 + 4) >> 3;
        } else {
            q0n = (2 * q1 + q0 + p1 + 2) >> 2;
            q1n = q1;
            q2n = q2;
        }

        p0 = p0n;
        if (strongA) {
            p1 = p1n;
            p2 = p2n;
        }

        q0 = q0n;
        if (strongB) {
            q1 = q1n;
            q2 = q2n;
        }
    }
}

void DeblockingFilter::filterChromaEdge(
    uint8_t indexA, uint8_t indexB, uint8_t bS,
    PixType &p3, PixType &p2, PixType &p1, PixType &p0,
    PixType &q0, PixType &q1, PixType &q2, PixType &q3) const
{
    uint8_t bitDepth = bitDepthC;
    PixType alpha = alphaMap[indexA] << (bitDepth - 8);
    PixType beta = betaMap[indexB] << (bitDepth - 8);
    bool filterSamplesFlag = bS != 0 && Abs<ResType>(p0 - q0) < alpha &&
        Abs<ResType>(p1 - p0) < beta && Abs<ResType>(q1 - q0) < beta;

    Unused(p3);
    Unused(p2);
    Unused(q2);
    Unused(q3);
    
    if (!filterSamplesFlag) {
        return;
    } else if (bS < 4) {
        PixType tc0 = tc0Map[bS - 1][indexA] << (bitDepth - 8);
        PixType tc = tc0 + 1;
        ResType delta = Clip3<ResType>(-tc, tc, (((q0 - p0) << 2) + (p1 - q1) + 4) >> 3);

        p0 = (PixType)Clip1<ResType>(p0 + delta, bitDepth);
        q0 = (PixType)Clip1<ResType>(q0 - delta, bitDepth);
    } else {
        PixType p0n;
        PixType q0n;

        p0n = (2 * p1 + p0 + q1 + 2) >> 2;
        q0n = (2 * q1 + q0 + p1 + 2) >> 2;

        p0 = p0n;
        q0 = q0n;
    }
}
