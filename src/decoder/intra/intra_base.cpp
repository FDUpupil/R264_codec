#include "intra_base.h"
#include <iostream>

#include <exception>

IntraBase::IntraBase(const PictureLevelConfig &cfgPic)
    : tq(
        (ChromaArrayType)cfgPic.chromaFormat, cfgPic.bitDepthY, cfgPic.bitDepthC,
        cfgPic.chromaQPIndexOffset, cfgPic.secondChromaQPIndexOffset),
      chromaFormat(cfgPic.chromaFormat),
      bitDepthY(cfgPic.bitDepthY), bitDepthC(cfgPic.bitDepthC),
      separateColourPlaneFlag(cfgPic.separateColourPlaneFlag),
      transform8x8ModeFlag(cfgPic.transform8x8ModeFlag)
{
    compCount = (uint8_t)(separateColourPlaneFlag ? COLOUR_COMPONENT_COUNT : 1);
    planeCount = (uint8_t)((chromaFormat != CHROMA_FORMAT_400) ? COLOUR_COMPONENT_COUNT : 1);

    pred4x4[COLOUR_COMPONENT_Y] = new Intra4x4Predictor(bitDepthY);
    pred8x8[COLOUR_COMPONENT_Y] = transform8x8ModeFlag ? new Intra8x8Predictor(bitDepthY) : nullptr;
    pred16x16[COLOUR_COMPONENT_Y] = new Intra16x16Predictor(bitDepthY);

    if (chromaFormat == CHROMA_FORMAT_444) {
        for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
            pred4x4[planeID] = new Intra4x4Predictor(bitDepthC);
            pred8x8[planeID] = transform8x8ModeFlag ? new Intra8x8Predictor(bitDepthC) : nullptr;
            pred16x16[planeID] = new Intra16x16Predictor(bitDepthC);
            predChroma[planeID - 1] = nullptr;
        }
    } else {
		if (chromaFormat != CHROMA_FORMAT_400)
			for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
				pred4x4[planeID] = nullptr;
				pred8x8[planeID] = nullptr;
				pred16x16[planeID] = nullptr;
				predChroma[planeID - 1] = new IntraChromaPredictor(chromaFormat, bitDepthC);
        }
    }

    tq.setScalingMatrices(*cfgPic.scalingMatrix4x4Intra, *cfgPic.scalingMatrix8x8Intra);
}

void IntraBase::init(const SliceLevelConfig &cfgSlic)
{
    sliceType = cfgSlic.sliceType;

    if (!separateColourPlaneFlag)
        tq.init(cfgSlic.sliceQP[COLOUR_COMPONENT_Y]);
    else
        tq.init(
            cfgSlic.sliceQP[COLOUR_COMPONENT_Y],
            cfgSlic.sliceQP[COLOUR_COMPONENT_CB],
            cfgSlic.sliceQP[COLOUR_COMPONENT_CR]);
    
    tq.setBiasFactor(QUANTIZER_FLAT_BIAS);

    if (!separateColourPlaneFlag) {
        lastMbQP[COLOUR_COMPONENT_Y] = cfgSlic.sliceQP[COLOUR_COMPONENT_Y];
    } else {
        lastMbQP[COLOUR_COMPONENT_Y]  = cfgSlic.sliceQP[COLOUR_COMPONENT_Y];
        lastMbQP[COLOUR_COMPONENT_CB] = cfgSlic.sliceQP[COLOUR_COMPONENT_CB];
        lastMbQP[COLOUR_COMPONENT_CR] = cfgSlic.sliceQP[COLOUR_COMPONENT_CR];
    }
}

void IntraBase::cycle(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbEncoded, const IntraToMemCtrl &memOut)
{
    start(mbInfo, memIn, mbEncoded, memOut);

    preprocess();
    estimate();
    decidePartition();
    encodeDecision();
    postprocess();

    finish(mbInfo, memIn, mbEncoded, memOut);
}

/// used for rec in decoder
void IntraBase::cycle(const MacroblockInfo &mbInfo, const MemCtrlToRec &memIn, EncodedMb *mbDec, const RecToMemCtrl &memOut)
{
    start(mbInfo, memIn, mbDec, memOut);

    preprocess_rec();
    setPredModes();
    reconstructure();


}

void IntraBase::start(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbEncoded, const IntraToMemCtrl &memOut)
{
    // MacroblockInfo
    xInMbs = mbInfo.xInMbs;
    yInMbs = mbInfo.yInMbs;
    neighbour = mbInfo.neighbour;
    mbQP[COLOUR_COMPONENT_Y] = mbInfo.mbQP[COLOUR_COMPONENT_Y];
    
    if (separateColourPlaneFlag) {
        mbQP[COLOUR_COMPONENT_CB] = mbInfo.mbQP[COLOUR_COMPONENT_CB];
        mbQP[COLOUR_COMPONENT_CR] = mbInfo.mbQP[COLOUR_COMPONENT_CR];
    }

    
    // MemCtrlToIntra
    orgMb = memIn.orgMb;
    refPixels = memIn.refPixels;
    refModes = memIn.refModes;

    // EncodedMb
    mbEnc = mbEncoded;

    // IntraToMemCtrl
    recMb = memOut.recMb;
    predModes = memOut.predModes;
}

///
void IntraBase::start(const MacroblockInfo &mbInfo, const MemCtrlToRec &memIn, EncodedMb *mbDec, const RecToMemCtrl &memOut)
{
    // MacroblockInfo
    xInMbs = mbInfo.xInMbs;
    yInMbs = mbInfo.yInMbs;
    neighbour = mbInfo.neighbour;
    
    mbQP[COLOUR_COMPONENT_Y] = mbDec[COLOUR_COMPONENT_Y].qp;
    if (separateColourPlaneFlag) {
        mbQP[COLOUR_COMPONENT_CB] = mbDec[COLOUR_COMPONENT_CB].qp;
        mbQP[COLOUR_COMPONENT_CR] = mbDec[COLOUR_COMPONENT_CR].qp;
    }

    // MemCtrToRec
    refPixels = memIn.refPixels;
    refModes = memIn.refModes;

    // DecodeMb
    mbEnc = mbDec;

    // RecToMemCtrl
    recMb = memOut.recMb;
    predModes = memOut.predModes;

}

void IntraBase::finish(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbEncoded, const IntraToMemCtrl &memOut)
{
    // Simply do nothing
    Unused(mbInfo);
    Unused(memIn);
    Unused(mbEncoded);
    Unused(memOut);

    // QUIRK: if mbtype != INTRA_16x16 && CodedBlockPatternLuma > 0 && CodedBlockPatternChroma > 0
    //        or mb_skip_flag is true, then mb_qp_delta is not coded and inferred to be equal to 0.
    //        Because mb_qp_delta is decided by SysCtrl/RateCtrl and TQ module has update new QP,
    //        however, old QP must be recovered to ensure correct QP from next macroblock. Due to
    //        completely zeros under this circumstance, reconstruct pixels are not affected.
    //        However, we shall tell SysCtrl/RateCtrl that QP is not coded at this macroblock, and
    //        QP in TQ module shall be recovered before encoding next macroblock. 
}

void IntraBase::preprocess()
{
    if (!separateColourPlaneFlag)
        tq.updateMacroblockQP(mbQP[COLOUR_COMPONENT_Y]);
    else
        tq.updateMacroblockQP(
            mbQP[COLOUR_COMPONENT_Y],
            mbQP[COLOUR_COMPONENT_CB],
            mbQP[COLOUR_COMPONENT_CR]
        );
    
    orgMb16x16 = &(*orgMb)[0];

    SplitBlock<16, 16>((*orgMb)[COLOUR_COMPONENT_Y], orgMb4x4[COLOUR_COMPONENT_Y]);
    if (transform8x8ModeFlag)
        SplitBlock<16, 16>((*orgMb)[COLOUR_COMPONENT_Y], orgMb8x8[COLOUR_COMPONENT_Y]);
    
    if (chromaFormat == CHROMA_FORMAT_444) {
        for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
            SplitBlock<16, 16>((*orgMb)[planeID], orgMb4x4[planeID]);
            if (transform8x8ModeFlag)
                SplitBlock<16, 16>((*orgMb)[planeID], orgMb8x8[planeID]);
        }
    }

    for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            neighbour4x4[yInSbs][xInSbs] = getNeighbourState4x4(xInSbs, yInSbs);

    for (uint8_t yInSbs = 0; yInSbs < 2; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 2; ++xInSbs)
            neighbour8x8[yInSbs][xInSbs] = getNeighbourState8x8(xInSbs, yInSbs);
    
    neighbour16x16 = getNeighbourState16x16();
    neighbourChroma = getNeighbourStateChroma();
}

///
void IntraBase::preprocess_rec()
{
    if (!separateColourPlaneFlag)
        tq.updateMacroblockQP(mbQP[COLOUR_COMPONENT_Y]);
    else
        tq.updateMacroblockQP(
            mbQP[COLOUR_COMPONENT_Y],
            mbQP[COLOUR_COMPONENT_CB],
            mbQP[COLOUR_COMPONENT_CR]
        );

    for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            neighbour4x4[yInSbs][xInSbs] = getNeighbourState4x4(xInSbs, yInSbs);

    for (uint8_t yInSbs = 0; yInSbs < 2; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 2; ++xInSbs)
            neighbour8x8[yInSbs][xInSbs] = getNeighbourState8x8(xInSbs, yInSbs);
    
    // for (uint8_t yInSbs = 0; yInSbs < 2; ++yInSbs)
    //     for (uint8_t xInSbs = 0; xInSbs < 2; ++xInSbs)
    //         printf("neighbour: %x\n", neighbour8x8[yInSbs][xInSbs]);

    neighbour16x16 = getNeighbourState16x16();
    neighbourChroma = getNeighbourStateChroma();
}

void IntraBase::postprocess()
{
    for (int compID = 0; compID < compCount; ++compID)
        encodePartition((ColourComponent)compID);
    for (int compID = 0; compID < compCount; ++compID)
        encodeTransformSize8x8Flag((ColourComponent)compID);
    for (int planeID = 0; planeID < planeCount; ++planeID)
        encodeNonZeroFlags((ColourComponent)planeID);
    for (int compID = 0; compID < compCount; ++compID)
        encodeQP((ColourComponent)compID);
}

inline int IntraBase::getNeighbourState4x4(uint8_t xInSbs, uint8_t yInSbs) const
{
    int nb = 0;

    if (xInSbs > 0)
        nb |= NEIGHBOUR_AVAILABLE_LEFT;
    else
        nb |= neighbour & NEIGHBOUR_AVAILABLE_LEFT;
    
    if (xInSbs == 0 && yInSbs == 0)
        nb |= neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT;
    else if (xInSbs == 0)
        nb |= (neighbour & NEIGHBOUR_AVAILABLE_LEFT) ? NEIGHBOUR_AVAILABLE_UP_LEFT : 0;
    else if (yInSbs == 0)
        nb |= (neighbour & NEIGHBOUR_AVAILABLE_UP) ? NEIGHBOUR_AVAILABLE_UP_LEFT : 0;
    else
        nb |= NEIGHBOUR_AVAILABLE_UP_LEFT;
    
    if (yInSbs > 0)
        nb |= NEIGHBOUR_AVAILABLE_UP;
    else
        nb |= neighbour & NEIGHBOUR_AVAILABLE_UP;
    
    if (yInSbs == 0) {
        if (xInSbs == 3)
            nb |= neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT;
        else
            nb |= (neighbour & NEIGHBOUR_AVAILABLE_UP) ? NEIGHBOUR_AVAILABLE_UP_RIGHT : 0;
    } else {
        if (xInSbs != 3 && !(xInSbs == 1 && yInSbs != 2))
            nb |= NEIGHBOUR_AVAILABLE_UP_RIGHT;
    }

    return nb;
}

void IntraBase::setRefPixels4x4(ColourComponent planeID, const Blk<PixType, 16, 16> &recComp, uint8_t xInSbs, uint8_t yInSbs)
{
    RefPixels ref;
    int nb = neighbour4x4[yInSbs][xInSbs];
    uint8_t xO = xInSbs * 4;
    uint8_t yO = yInSbs * 4;

    if (nb & NEIGHBOUR_AVAILABLE_LEFT) {
        if (xO == 0)
            for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
                ref.l[yOfSbs] = refPixels[planeID].l[yO + yOfSbs];
        else
            for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
                ref.l[yOfSbs] = recComp[yO + yOfSbs][xO - 1];
    }

    if (nb & NEIGHBOUR_AVAILABLE_UP_LEFT) {
        if (xO == 0 && yO == 0)
            ref.ul = refPixels[planeID].ul;
        else if (xO == 0)
            ref.ul = refPixels[planeID].l[yO - 1];
        else if (yO == 0)
            ref.ul = refPixels[planeID].u[xO - 1];
        else
            ref.ul = recComp[yO - 1][xO - 1];
    }

    if (nb & NEIGHBOUR_AVAILABLE_UP) {
        uint8_t xLen = (nb & NEIGHBOUR_AVAILABLE_UP_RIGHT) ? 8 : 4;
        if (yO == 0)
            for (uint8_t xOfSbs = 0; xOfSbs < xLen; ++xOfSbs)
                ref.u[xOfSbs] = refPixels[planeID].u[xO + xOfSbs];
        else
            for (uint8_t xOfSbs = 0; xOfSbs < xLen; ++xOfSbs)
                ref.u[xOfSbs] = recComp[yO - 1][xO + xOfSbs];
    }
    
    //printf("neighbour : %x \n", nb);
    pred4x4[planeID]->setRefPixels(nb, ref);
}

void IntraBase::setRefPixels8x8(ColourComponent planeID, const Blk<PixType, 16, 16> &recComp, uint8_t xInSbs, uint8_t yInSbs)
{
    RefPixels ref;
    int nb = neighbour8x8[yInSbs][xInSbs];
    uint8_t xO = xInSbs * 8;
    uint8_t yO = yInSbs * 8;

    if (nb & NEIGHBOUR_AVAILABLE_LEFT) {
        if (xO == 0)
            for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
                ref.l[yOfSbs] = refPixels[planeID].l[yO + yOfSbs];
        else
            for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
                ref.l[yOfSbs] = recComp[yO + yOfSbs][xO - 1];
    }

    if (nb & NEIGHBOUR_AVAILABLE_UP_LEFT) {
        if (xO == 0 && yO == 0)
            ref.ul = refPixels[planeID].ul;
        else if (xO == 0)
            ref.ul = refPixels[planeID].l[yO - 1];
        else if (yO == 0)
            ref.ul = refPixels[planeID].u[xO - 1];
        else
            ref.ul = recComp[yO - 1][xO - 1];
    }

    if (nb & NEIGHBOUR_AVAILABLE_UP) {
        uint8_t xLen = (nb & NEIGHBOUR_AVAILABLE_UP_RIGHT) ? 16 : 8;
        if (yO == 0)
            for (uint8_t xOfSbs = 0; xOfSbs < xLen; ++xOfSbs)
                ref.u[xOfSbs] = refPixels[planeID].u[xO + xOfSbs];
        else
            for (uint8_t xOfSbs = 0; xOfSbs < xLen; ++xOfSbs)
                ref.u[xOfSbs] = recComp[yO - 1][xO + xOfSbs];
    }
    //printf("neighbour : %x \n", nb);
    pred8x8[planeID]->setRefPixels(nb, ref);
}

void IntraBase::setRefPixels16x16(ColourComponent planeID)
{
    pred16x16[planeID]->setRefPixels(neighbour16x16, refPixels[planeID]);
}

void IntraBase::setRefPixelsChroma(ColourComponent planeID)
{
    predChroma[planeID - 1]->setRefPixels(neighbourChroma, refPixels[planeID]);
}

void IntraBase::encodeDecision()
{
    if (!separateColourPlaneFlag)
        IntraBaseCheck(
            transform8x8ModeFlag ||
            partition[COLOUR_COMPONENT_Y] != INTRA_PARTITION_8x8);
    else
        IntraBaseCheck(
            transform8x8ModeFlag ||
            (partition[COLOUR_COMPONENT_Y] != INTRA_PARTITION_8x8 &&
            partition[COLOUR_COMPONENT_CB] != INTRA_PARTITION_8x8 &&
            partition[COLOUR_COMPONENT_CR] != INTRA_PARTITION_8x8));
    
    encodeDecisionLuma(COLOUR_COMPONENT_Y);
    if (chromaFormat != CHROMA_FORMAT_400) {
        if (chromaFormat != CHROMA_FORMAT_444) {
            encodeDecisionChroma();
        } else {
            encodeDecisionLuma(COLOUR_COMPONENT_CB);
            encodeDecisionLuma(COLOUR_COMPONENT_CR);
        }
    }
}

void IntraBase::encodeDecision4x4(ColourComponent planeID)
{
    IntraBaseCheck(
        planeID == COLOUR_COMPONENT_Y ||
        chromaFormat == CHROMA_FORMAT_444);
    
    if (planeID == COLOUR_COMPONENT_Y || separateColourPlaneFlag)
        encodeIntraModes4x4(planeID);
    encodeCoefs4x4(planeID, (*recMb)[planeID], mbEnc[planeID].coefs);
}

void IntraBase::encodeDecision8x8(ColourComponent planeID)
{
    IntraBaseCheck(
        planeID == COLOUR_COMPONENT_Y ||
        chromaFormat == CHROMA_FORMAT_444);

    if (planeID == COLOUR_COMPONENT_Y || separateColourPlaneFlag)
        encodeIntraModes8x8(planeID);
    encodeCoefs8x8(planeID, (*recMb)[planeID], mbEnc[planeID].coefs);
}

void IntraBase::encodeDecision16x16(ColourComponent planeID)
{
    IntraBaseCheck(
        planeID == COLOUR_COMPONENT_Y ||
        chromaFormat == CHROMA_FORMAT_444);
    
    if (planeID == COLOUR_COMPONENT_Y || separateColourPlaneFlag)
        encodeIntraModes16x16(planeID);
    encodeCoefs16x16(planeID, (*recMb)[planeID], mbEnc[planeID].coefs);
}

void IntraBase::encodeDecisionLuma(ColourComponent planeID)
{
    MacroblockPartition part = separateColourPlaneFlag ? partition[planeID] : partition[COLOUR_COMPONENT_Y];

    switch (part) {
    case INTRA_PARTITION_4x4:
        encodeDecision4x4(planeID);
        break;

    case INTRA_PARTITION_8x8:
        encodeDecision8x8(planeID);
        break;

    case INTRA_PARTITION_16x16:
        encodeDecision16x16(planeID);
        break;
    }
}

void IntraBase::encodeDecisionChroma()
{
    IntraBaseCheck(
        chromaFormat == CHROMA_FORMAT_420 ||
        chromaFormat == CHROMA_FORMAT_422);

    encodeIntraModesChroma();
    encodeCoefsChroma(COLOUR_COMPONENT_CB, (*recMb)[COLOUR_COMPONENT_CB], mbEnc[COLOUR_COMPONENT_CB].coefs);
    encodeCoefsChroma(COLOUR_COMPONENT_CR, (*recMb)[COLOUR_COMPONENT_CR], mbEnc[COLOUR_COMPONENT_CR].coefs);
}

void IntraBase::encodeIntraModes4x4(ColourComponent compID)
{
    CopyBlock(predModes4x4[compID], predModes[compID]);

    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;
        int predIntra4x4PredMode = getPredIntra4x4PredMode(compID, xInSbs, yInSbs);

        if (predIntra4x4PredMode == predModes4x4[compID][yInSbs][xInSbs]) {
            mbEnc[compID].prevIntraPredModeFlag[idx] = 1;
            mbEnc[compID].remIntraPredMode[idx] = 0xFF;
        } else {
            mbEnc[compID].prevIntraPredModeFlag[idx] = 0;
            mbEnc[compID].remIntraPredMode[idx] = (uint8_t)((predModes4x4[compID][yInSbs][xInSbs] < predIntra4x4PredMode) ?
                predModes4x4[compID][yInSbs][xInSbs] : predModes4x4[compID][yInSbs][xInSbs] - 1);
        }
    }
}

void IntraBase::encodeIntraModes8x8(ColourComponent compID)
{
    for (uint8_t yInSbs = 0; yInSbs < 2; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 2; ++xInSbs)
            predModes[compID][yInSbs * 2 + 0][xInSbs * 2 + 0] =
            predModes[compID][yInSbs * 2 + 0][xInSbs * 2 + 1] =
            predModes[compID][yInSbs * 2 + 1][xInSbs * 2 + 0] =
            predModes[compID][yInSbs * 2 + 1][xInSbs * 2 + 1] = predModes8x8[compID][yInSbs][xInSbs];
    
    for (uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;
        int predIntra8x8PredMode = getPredIntra8x8PredMode(compID, xInSbs, yInSbs);

        if (predIntra8x8PredMode == predModes8x8[compID][yInSbs][xInSbs]) {
            mbEnc[compID].prevIntraPredModeFlag[idx] = 1;
            mbEnc[compID].remIntraPredMode[idx] = 0xFF;
        } else {
            mbEnc[compID].prevIntraPredModeFlag[idx] = 0;
            mbEnc[compID].remIntraPredMode[idx] = (uint8_t)((predModes8x8[compID][yInSbs][xInSbs] < predIntra8x8PredMode) ?
                predModes8x8[compID][yInSbs][xInSbs] : predModes8x8[compID][yInSbs][xInSbs] - 1);
        }
    }
}

void IntraBase::encodeIntraModes16x16(ColourComponent compID)
{
    for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            predModes[compID][yInSbs][xInSbs] = INTRA_4x4_DC;
    
    mbEnc[compID].intra16x16PredMode = (uint8_t)predMode16x16[compID];
}

void IntraBase::encodeIntraModesChroma()
{
    mbEnc[COLOUR_COMPONENT_Y].intraChromaPredMode = (uint8_t)predModeChroma;
}

void IntraBase::encodeCoefs4x4(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    Intra4x4Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> resBlk;
    uint8_t bitDepth = (planeID == 0) ? bitDepthY : bitDepthC;
    ColourComponent compID = separateColourPlaneFlag ? planeID : COLOUR_COMPONENT_Y;

    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;

        setRefPixels4x4(planeID, recComp, xInSbs, yInSbs);
        pred4x4[planeID]->predict(predModes4x4[compID][yInSbs][xInSbs], predBlk);

        DiffBlock(orgMb4x4[planeID][yInSbs][xInSbs], predBlk, resBlk);

        tq.encode4x4(planeID, resBlk, coefs.blk4x4[idx], resBlk);

        for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
            for (uint8_t xOfSbs = 0; xOfSbs < 4; ++xOfSbs)
                recComp[yInSbs * 4 + yOfSbs][xInSbs * 4 + xOfSbs] =
                    (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + resBlk[yOfSbs][xOfSbs], bitDepth);
    }
}

void IntraBase::encodeCoefs8x8(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    Intra8x8Predictor::PredBlk predBlk;
    Blk<ResType, 8, 8> resBlk;
    uint8_t bitDepth = (planeID == 0) ? bitDepthY : bitDepthC;
    ColourComponent compID = separateColourPlaneFlag ? planeID : COLOUR_COMPONENT_Y;

    for (uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;

        setRefPixels8x8(planeID, recComp, xInSbs, yInSbs);
        pred8x8[planeID]->predict(predModes8x8[compID][yInSbs][xInSbs], predBlk);

        DiffBlock(orgMb8x8[planeID][yInSbs][xInSbs], predBlk, resBlk);

        tq.encode8x8(planeID, resBlk, coefs.blk4x4[idx], resBlk);

        for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
            for (uint8_t xOfSbs = 0; xOfSbs < 8; ++xOfSbs)
                recComp[yInSbs * 2 + yOfSbs][xInSbs * 2 + xOfSbs] =
                    (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + resBlk[yOfSbs][xOfSbs], bitDepth);
    }
}

void IntraBase::encodeCoefs16x16(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    Intra16x16Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> resBlk[4][4];
    uint8_t bitDepth = (planeID == 0) ? bitDepthY : bitDepthC;
    ColourComponent compID = separateColourPlaneFlag ? planeID : COLOUR_COMPONENT_Y;

    // We assume that reference pixels have been set
    // setRefPixels16x16(planeID);
    pred16x16[planeID]->predict(predMode16x16[compID], predBlk);

    SplitBlock<16, 16>(orgMb16x16[planeID], predBlk, resBlk);

    tq.encode16x16(planeID, resBlk, coefs.blk16x16, resBlk);

    SpliceBlock<16, 16>(predBlk, resBlk, recComp, bitDepth);
}

void IntraBase::encodeCoefsChroma(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    IntraChromaPredictor::PredBlk predBlk;
    IntraChromaPredMode predMode;

    predMode = predModeChroma;
    // We assume that reference pixels have been set
    // setRefPixelsChroma(planeID);
    predChroma[planeID - 1]->predict(predMode, predBlk);

    if (chromaFormat == CHROMA_FORMAT_420) {
        Blk<ResType, 4, 4> resBlk[2][2];

        SplitBlock<8, 8>(orgMb16x16[planeID], predBlk, resBlk);

        tq.encodeChroma(planeID, resBlk, coefs.blk16x16, resBlk);

        SpliceBlock<8, 8>(predBlk, resBlk, recComp, bitDepthC);
    } else {
        Blk<ResType, 4, 4> resBlk[4][2];

        SplitBlock<8, 16>(orgMb16x16[planeID], predBlk, resBlk);

        tq.encodeChroma(planeID, resBlk, coefs.blk16x16, resBlk);

        SpliceBlock<8, 16>(predBlk, resBlk, recComp, bitDepthC);
    }
}

void IntraBase::encodePartition(ColourComponent compID)
{
    mbEnc[compID].mbPart = partition[compID];
}

void IntraBase::encodeTransformSize8x8Flag(ColourComponent compID)
{
    mbEnc[compID].transformSize8x8Flag = partition[compID] == INTRA_PARTITION_8x8;
}

void IntraBase::encodeNonZeroFlags(ColourComponent planeID)
{
    if (planeID == COLOUR_COMPONENT_Y || chromaFormat == CHROMA_FORMAT_444) {
        int compID = separateColourPlaneFlag ? planeID : COLOUR_COMPONENT_Y;
        uint32_t flag = 0;

        switch (partition[compID]) {
        case INTRA_PARTITION_4x4:
            for (uint8_t idx = 0; idx < 16; ++idx)
                flag |= checkIfNonZero(mbEnc[planeID].coefs.blk4x4[idx], 16) << idx;
            break;

        case INTRA_PARTITION_8x8:
            for (uint8_t idx = 0; idx < 4; ++idx)
                flag |= (checkIfNonZero(mbEnc[planeID].coefs.blk8x8[idx], 64) ? 0xF : 0) << (idx * 4);
            break;

        case INTRA_PARTITION_16x16:
            for (uint8_t idx = 0; idx < 16; ++idx)
                flag |= checkIfNonZero(mbEnc[planeID].coefs.blk16x16.ac[idx], 15) << idx;
            flag |= checkIfNonZero(mbEnc[planeID].coefs.blk16x16.dc, 16) << NON_ZERO_FLAG_DC_SHIFT;
            break;
        }

        mbEnc[planeID].nonZeroFlags = flag;
    } else {
        uint8_t count4x4 = (chromaFormat == CHROMA_FORMAT_420) ? 4 : 8;
        uint32_t flag = 0;

        for (uint8_t idx = 0; idx < count4x4; ++idx)
            flag |= checkIfNonZero(mbEnc[planeID].coefs.blk16x16.ac[idx], 15) << idx;
        flag |= checkIfNonZero(mbEnc[planeID].coefs.blk16x16.dc, count4x4) << NON_ZERO_FLAG_DC_SHIFT;

        mbEnc[planeID].nonZeroFlags = flag;
    }
}

void IntraBase::encodeQP(ColourComponent compID)
{
    bool qpUpdated;

    if (partition[compID] == INTRA_PARTITION_16x16)
        qpUpdated = true;
    else if (separateColourPlaneFlag || chromaFormat == CHROMA_FORMAT_400)
        qpUpdated = !!mbEnc[compID].nonZeroFlags;
    else
        qpUpdated = mbEnc[COLOUR_COMPONENT_Y].nonZeroFlags  ||
                    mbEnc[COLOUR_COMPONENT_CB].nonZeroFlags ||
                    mbEnc[COLOUR_COMPONENT_CR].nonZeroFlags;
    
    if (qpUpdated) {
        mbEnc[compID].qp = mbQP[compID];
        lastMbQP[compID] = mbQP[compID];
    } else {
        mbEnc[compID].qp = lastMbQP[compID];
    }

    mbEnc[compID].qpUpdated = qpUpdated;
}

void IntraBase::setPredModes()
{
    switch(mbEnc[COLOUR_COMPONENT_Y].mbPart) {
        case INTRA_PARTITION_4x4:
            setPredModes4x4(COLOUR_COMPONENT_Y);
            break;

        case INTRA_PARTITION_8x8:
            setPredModes8x8(COLOUR_COMPONENT_Y);
            break;

        case INTRA_PARTITION_16x16:
            setPredModes16x16(COLOUR_COMPONENT_Y);
            break;
    }
    if(separateColourPlaneFlag) {
        for(int planeID = COLOUR_COMPONENT_CB; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
            switch(mbEnc[planeID].mbPart) {
                case INTRA_PARTITION_4x4:
                    setPredModes4x4((ColourComponent)planeID);
                    break;

                case INTRA_PARTITION_8x8:
                    setPredModes8x8((ColourComponent)planeID);
                    break;

                case INTRA_PARTITION_16x16:
                    setPredModes16x16((ColourComponent)planeID);
                    break;
            }
        }
    } 
}

void IntraBase::setPredModes4x4(ColourComponent planeID)
{
    for(uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;
        int predIntra4x4PredMode = getPredIntra4x4PredMode(planeID, xInSbs, yInSbs); 
        //printf("predmode : %d \n", predIntra4x4PredMode);

        if(mbEnc[planeID].prevIntraPredModeFlag[idx])
            predModes4x4[planeID][yInSbs][xInSbs] = (Intra4x4PredMode)predIntra4x4PredMode;
        else
            predModes4x4[planeID][yInSbs][xInSbs] = (Intra4x4PredMode)((mbEnc[planeID].remIntraPredMode[idx] < (uint8_t)predIntra4x4PredMode) ?
                mbEnc[planeID].remIntraPredMode[idx] : mbEnc[planeID].remIntraPredMode[idx] + 1);
        //printf("%d submb remmode is %d\n", idx, mbEnc[planeID].remIntraPredMode[idx]);
        //printf("%d submb mode is %d \n", idx, predModes4x4[planeID][yInSbs][xInSbs]);
        // restore for next processes
        predModes[planeID][yInSbs][xInSbs] = predModes4x4[planeID][yInSbs][xInSbs];
    }

}

void IntraBase::setPredModes8x8(ColourComponent planeID)
{
    for(uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;
        int predIntra8x8PredMode = getPredIntra8x8PredMode(planeID, xInSbs, yInSbs);
        //printf("predmode : %d \n", predIntra8x8PredMode);

        if(mbEnc[planeID].prevIntraPredModeFlag[idx])
            predModes8x8[planeID][yInSbs][xInSbs] = (Intra8x8PredMode)predIntra8x8PredMode;
        else
            predModes8x8[planeID][yInSbs][xInSbs] = (Intra8x8PredMode)((mbEnc[planeID].remIntraPredMode[idx] < (uint8_t)predIntra8x8PredMode) ?
                mbEnc[planeID].remIntraPredMode[idx] : mbEnc[planeID].remIntraPredMode[idx] + 1);
        //printf("%d submb remmode is %d\n", idx, mbEnc[planeID].remIntraPredMode[idx]);
        //printf("%d submb mode is %d \n", idx, predModes8x8[planeID][yInSbs][xInSbs]);
        // restore for next processes
        predModes[planeID][yInSbs * 2 + 0][xInSbs * 2 + 0] =
            predModes[planeID][yInSbs * 2 + 0][xInSbs * 2 + 1] =
            predModes[planeID][yInSbs * 2 + 1][xInSbs * 2 + 0] =
            predModes[planeID][yInSbs * 2 + 1][xInSbs * 2 + 1] = predModes8x8[planeID][yInSbs][xInSbs];
    }
}

void IntraBase::setPredModes16x16(ColourComponent planeID)
{
    for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            predModes[planeID][yInSbs][xInSbs] = INTRA_4x4_DC;

    predMode16x16[planeID] = (Intra16x16PredMode)mbEnc[planeID].intra16x16PredMode;
}
