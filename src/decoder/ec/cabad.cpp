#include"cabad.h"

CABAD::CABAD(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg) : EntropyDecoder(seqCfg, picCfg)
{
    if(chromaFormat == CHROMA_FORMAT_444 && separateColourPlaneFlag) {
        ctxModel = new ContextModelSet[COLOUR_COMPONENT_COUNT];
        engine = new  ArithDecEngine[COLOUR_COMPONENT_COUNT];
    } else {
        ctxModel = new ContextModelSet[1];
        engine = new  ArithDecEngine[1];
    }

}

CABAD::~CABAD()
{
    delete[] ctxModel;
    delete[] engine;
}

void CABAD::init(const SliceLevelConfig &sliCfg, Bitstream *bs)
{
    EntropyDecoder::init(sliCfg);

	// Bitstream
    for(int compID = 0; compID < compCount; ++compID) {
        engine[compID].setBitstream(bs + compID);
        if (sliceType == I_SLICE)
            ctxModel[compID].init(sliceQP);
        else
            ctxModel[compID].init(sliceQP, sliCfg.cabacInitIdc);
    }

    for(int compID = 0; compID < compCount; ++compID)
        engine[compID].init();
}

void CABAD::start(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut)
{
    EntropyDecoder::start(mbInfo, mbDec, memIn, memOut);

    refFlags = memIn.cabacRefFlags;

    curFlags = memOut.cabacCurFlags;
}

void CABAD::finish(MacroblockInfo& mbInfo, EncodedMb* mbDec, MemCtrlToEC& memIn, ECToMemCtrl& memOut)
{
	EntropyDecoder::finish(mbInfo, mbDec, memIn, memOut);
}

void CABAD::preprocess()
{
	EntropyDecoder::preprocess();
}

void CABAD::postprocess()
{
	EntropyDecoder::postprocess();
    for (int planeID = 0; planeID < planeCount; ++planeID)
        updateCurFlags((ColourComponent)planeID);
}

void CABAD::readSliceDataInfos(ColourComponent compID)
{
	EntropyDecoder::readSliceDataInfos(compID);

	readEndOfSliceFlag(compID);
}

void CABAD::readMbTypeInfos(ColourComponent compID) /// read 1~7 bits
{
    uint8_t     binVal;
    uint16_t    ctxIdx;

    uint8_t     cbpChromaDCFlag;
    uint8_t     cbpChromaACFlag = 0;
    
    ctxIdx = ContextModelSet::getCtxIdxForMbType(sliceType, refFlags[COLOUR_COMPONENT_Y].nonIntraNxN, 0);

    binVal = biariDecodeRegular(compID, ctxIdx);

    if(binVal == 0){
        mb[compID].mbPart =  INTRA_PARTITION_4x4;
    }
    else {
        mb[compID].mbPart =  INTRA_PARTITION_16x16;

         mb[compID].transformSize8x8Flag = 0;

        binVal = biariDecodeTerminate(compID);
        //cbpLuma AC
        ctxIdx = ContextModelSet::getCtxIdxForMbType(sliceType, refFlags[compID].nonIntraNxN, 2);
        cbpLumaACFlag = biariDecodeRegular(compID, ctxIdx);
		cbpLuma[compID] = cbpLumaACFlag ? 0xF : 0x0;
        //cbpChroma
        ctxIdx = ContextModelSet::getCtxIdxForMbType(sliceType, refFlags[compID].nonIntraNxN, 3);
        cbpChromaDCFlag = biariDecodeRegular(compID, ctxIdx);

        if(cbpChromaDCFlag != 0){
            ctxIdx ++;
            cbpChromaACFlag = biariDecodeRegular(compID, ctxIdx);
        }
        cbpChroma = cbpChromaDCFlag << 1 | cbpChromaACFlag;
        //16x16PreMode
        ctxIdx = ContextModelSet::getCtxIdxForMbType(sliceType, refFlags[compID].nonIntraNxN, 5);
        binVal = biariDecodeRegular(compID, ctxIdx);

        ctxIdx++;
        mb[compID].intra16x16PredMode = binVal << 1 + biariDecodeRegular(compID, ctxIdx);
    }

}

void CABAD::readTransformSize8x8FlagInfos(ColourComponent compID) /// read 1 bit
{
    uint8_t     binVal;
    uint16_t    ctxIdx;

    ctxIdx = ContextModelSet::getCtxIdxForTransformSize8x8Flag(refFlags[compID].transform8x8);
    binVal = biariDecodeRegular(compID, ctxIdx);

    mb[compID].transformSize8x8Flag = binVal;
    mb[compID].mbPart = (binVal == 1) ? INTRA_PARTITION_8x8 : INTRA_PARTITION_4x4;

}

void CABAD::readIntraNxNPredModeInfos(ColourComponent compID) /// read modeCount * 1~4 bits
{
    uint8_t modeCount = (mb[compID].mbPart == INTRA_PARTITION_4x4) ? 16 : 4;
    uint16_t ctxIdx;
    for (int idx = 0; idx < modeCount; ++idx) {
        ctxIdx = ContextModelSet::getCtxIdxForPrevIntraNxNPredModeFlag();
        mb[compID].prevIntraPredModeFlag[idx] = biariDecodeRegular(compID, ctxIdx);

        if (!mb[compID].prevIntraPredModeFlag[idx]){
            ctxIdx = ContextModelSet::getCtxIdxForRemIntraNxNPredMode();
            mb[compID].remIntraPredMode[idx] = biariDecodeFixedlLength(compID, 3, ctxIdx);
        }    
    }
}

void CABAD::readIntraChromaPredMode() ///read 1 or 3 bits
{
    uint8_t     binVal;
    uint32_t    binsVal;
    uint16_t    ctxIdx;

    ctxIdx =  ContextModelSet::getCtxIdxForIntraChromaPredMode(refFlags[COLOUR_COMPONENT_Y].intraChromaPredMode, 0);
    binVal = biariDecodeRegular(COLOUR_COMPONENT_Y, ctxIdx);
    if(binVal == 0) {
        mb[COLOUR_COMPONENT_Y].intraChromaPredMode = 0;
    }
    else {
        ctxIdx = ContextModelSet::getCtxIdxForIntraChromaPredMode(refFlags[COLOUR_COMPONENT_Y].intraChromaPredMode, 1);
        binsVal = biariDecodeTruncatedUnary(COLOUR_COMPONENT_Y, 2, ctxIdx, ctxIdx);
        mb[COLOUR_COMPONENT_Y].intraChromaPredMode = (uint8_t)binsVal + 1;
    }
}

void CABAD::readCodedBlockPatternInfos(ColourComponent compID)
{
    readCodedBlockPatternLuma(compID); /// read 4 bits 
    if(chromaFormat == CHROMA_FORMAT_420 || chromaFormat == CHROMA_FORMAT_422)
        readCodedBlockPatternChroma();/// read 1 or 2 bits

}

void CABAD::readCodedBlockPatternLuma(ColourComponent compID)
  {
    uint8_t     refA0 = refFlags[COLOUR_COMPONENT_Y].cbpLuma.a[0];
    uint8_t     refA1 = refFlags[COLOUR_COMPONENT_Y].cbpLuma.a[1];
    uint8_t     refB0 = refFlags[COLOUR_COMPONENT_Y].cbpLuma.b[0];
    uint8_t     refB1 = refFlags[COLOUR_COMPONENT_Y].cbpLuma.b[1]; 

    uint16_t    ctxIdx00;
    uint16_t    ctxIdx01;  
    uint16_t    ctxIdx10;
    uint16_t    ctxIdx11;

    uint8_t     cur00;
    uint8_t     cur01;
    uint8_t     cur10;
    uint8_t     cur11;

    ctxIdx00 = ContextModelSet::getCtxIdxForCodedBlockPatternLuma(CordTermFlag2D<1>(refA0, refB0));
    cur00 = biariDecodeRegular(compID, ctxIdx00);

    ctxIdx01 = ContextModelSet::getCtxIdxForCodedBlockPatternLuma(CordTermFlag2D<1>(cur00, refB1));
    ctxIdx10 = ContextModelSet::getCtxIdxForCodedBlockPatternLuma(CordTermFlag2D<1>(refA1, cur00));

    cur01 = biariDecodeRegular(compID, ctxIdx01);
    cur10 = biariDecodeRegular(compID, ctxIdx10);

    ctxIdx11 = ContextModelSet::getCtxIdxForCodedBlockPatternLuma(CordTermFlag2D<1>(cur10, cur01));

    cur11 = biariDecodeRegular(compID, ctxIdx11);

    cbpLuma[compID] = (cur00 << 0) | (cur01 << 1) | (cur10 << 2) | (cur11 << 3);
}

void CABAD::readCodedBlockPatternChroma()
{
    uint8_t a = refFlags[COLOUR_COMPONENT_Y].cbpChroma.a;
    uint8_t b = refFlags[COLOUR_COMPONENT_Y].cbpChroma.b;

    uint8_t     binVal;
    uint16_t    ctxIdx;

    ctxIdx = ContextModelSet::getCtxIdxForCodedBlockPatternChroma(0, CordTermFlag2D<1>(!!a, !!b));
    binVal = biariDecodeRegular(COLOUR_COMPONENT_Y, ctxIdx);

    if(binVal == 0) {
        cbpChroma = 0;
    }
    else {
        cbpChroma = 0x2;
        ctxIdx = ContextModelSet::getCtxIdxForCodedBlockPatternChroma(1, CordTermFlag2D<1>(a & 1, b & 1));
        binVal = biariDecodeRegular(COLOUR_COMPONENT_Y, ctxIdx);
        cbpChroma |=  binVal;
    }
}

void CABAD::readMbQPDeltaInfos(ColourComponent compID)
{
    uint8_t     binVal;

    uint16_t    ctxIdx;

    ctxIdx = ContextModelSet::getCtxIdxForMbQPDelta(refFlags[compID].mbQPDelta, 0);
    binVal = biariDecodeRegular(compID, ctxIdx);

    if(binVal == 0) {
        mbQPDelta = 0;
    }
    else {
        mbQPDelta = biariDecodeUnary(compID, ctxIdx, ctxIdx+1) + 1;
    }
	if (xInMbs == 0 && yInMbs == 0)
		mb[compID].qp = sliceQP + mbQPDelta;
	else
		mb[compID].qp = lastMbQP + mbQPDelta;

	lastMbQP = mb[compID].qp;
}

void CABAD::readLumaCoefsInfos(ColourComponent compID, ColourComponent planeID)
{
    ColourComponent ctxCompID = separateColourPlaneFlag ? COLOUR_COMPONENT_Y : planeID;
    uint16_t    ctxIdx;
    uint8_t     flag;
    if(mb[compID].mbPart == INTRA_PARTITION_16x16) {
        // Decode DC coefficients
        ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                ctxCompID, CODED_BLOCK_LUMA_DC,
                getFlagForCodedBlockFlagLumaDC(planeID));
        flag = biariDecodeRegular(compID, ctxIdx);
        mb[planeID].nonZeroFlags |= flag << NON_ZERO_FLAG_DC_SHIFT;
        if(flag)
            readBlkCoefs(compID, ctxCompID, CODED_BLOCK_LUMA_DC, mb[planeID].coefs.blk16x16.dc);

        // Decode AC coefficient;
        if(cbpLuma[compID])
            for(uint8_t idx = 0; idx < 16; ++idx) {
                ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                    ctxCompID, CODED_BLOCK_LUMA_AC,
                    getFlagForCodedBlockFlagLumaAC(planeID, idx));
                flag = biariDecodeRegular(compID, ctxIdx);
                mb[planeID].nonZeroFlags |= flag << idx;
                if(flag)
                    readBlkCoefs(compID, ctxCompID, CODED_BLOCK_LUMA_AC, mb[planeID].coefs.blk16x16.ac[idx]);   
            }   
    }
    else if(!mb[compID].transformSize8x8Flag) {
        for (uint8_t i8x8 = 0; i8x8 < 4; ++i8x8)
            if (cbpLuma[compID] & 1 << i8x8)
                for (uint8_t i4x4 = 0; i4x4 < 4; ++i4x4) {
                    uint8_t idx = i8x8 << 2 | i4x4;
					printf("!read the %d sub mb luma!", i8x8*4+i4x4);
                    ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                        ctxCompID, CODED_BLOCK_LUMA_4x4,
                        getFlagForCodedBlockFlagLuma4x4(planeID, idx));
                    flag = biariDecodeRegular(compID, ctxIdx);
                    mb[planeID].nonZeroFlags |= flag << idx;
                    if(flag)
                      readBlkCoefs(compID, ctxCompID, CODED_BLOCK_LUMA_4x4, mb[planeID].coefs.blk4x4[idx]);   
                }
    }
    else {
        for (uint8_t i8x8 = 0; i8x8 < 4; ++i8x8) {
            printf("!read the %d sub mb luma!", i8x8);
            if (cbpLuma[compID] & 1 << i8x8) {
                flag = 1;
                if(chromaFormat == CHROMA_FORMAT_444 && !separateColourPlaneFlag) {
                    ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                        ctxCompID, CODED_BLOCK_LUMA_8x8,
                        getFlagForCodedBlockFlagLuma8x8(planeID, i8x8));
                    flag = biariDecodeRegular(compID, ctxIdx);
                    mb[planeID].nonZeroFlags |=  (flag? 0xF : 0x0) << (i8x8 * 4);
                }
                else {
                    mb[planeID].nonZeroFlags |=   0xF << (i8x8 * 4); // 8x8 nozeroflag info already in the cbpluma
                }
            if(flag) 
                readBlkCoefs(compID, ctxCompID, CODED_BLOCK_LUMA_8x8, mb[planeID].coefs.blk8x8[i8x8]);   
            }
        }
    }
}

void CABAD::readChromaCoefsInfos()
{
    uint8_t count4x4 = (chromaFormat == CHROMA_FORMAT_420) ? 4 : 8;
    uint16_t    ctxIdx;
    uint8_t     flag;

    if(cbpChroma != 0) {
    // Decode DC coefficient
		printf("!read the chroma DC!");
        for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
            ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                    (ColourComponent)planeID, CODED_BLOCK_CHROMA_DC,
                    getFlagForCodedBlockFlagChromaDC((ColourComponent)planeID));
            flag = biariDecodeRegular(COLOUR_COMPONENT_Y, ctxIdx);
            mb[planeID].nonZeroFlags |= flag << NON_ZERO_FLAG_DC_SHIFT;
            if(flag)
                readBlkCoefs(COLOUR_COMPONENT_Y, (ColourComponent)planeID, CODED_BLOCK_CHROMA_DC, mb[planeID].coefs.blk16x16.dc);
        }
    }

    if(cbpChroma == 0x3) {
        for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID)
            for (uint8_t idx = 0; idx < count4x4; ++idx) {
				printf("!read the %d chroma AC!", idx);
                ctxIdx = ContextModelSet::getCtxIdxForCodedBlockFlag(
                    (ColourComponent)planeID, CODED_BLOCK_CHROMA_AC,
                    getFlagForCodedBlockFlagChromaAC((ColourComponent)planeID, idx));
                flag = biariDecodeRegular(COLOUR_COMPONENT_Y, ctxIdx);
                mb[planeID].nonZeroFlags |= flag << idx;
                if(flag)
                    readBlkCoefs(COLOUR_COMPONENT_Y, (ColourComponent)planeID, CODED_BLOCK_CHROMA_AC, mb[planeID].coefs.blk16x16.ac[idx]);
          }  
    }
}

void CABAD::readBlkCoefs(ColourComponent compID, ColourComponent ctxCompID, CodedBlockType type, CoefType *coefs)
{
    uint8_t     noZeroLevelCount;
    uint8_t     maxLevelCount;
    uint16_t    ctxIdxOffsetForSCF;
    uint16_t    ctxIdxOffsetForLSCF;
    uint16_t    ctxIdxOffsetForCoeff;
    uint8_t     ctxIdxInc;
    uint16_t    ctxIdx;
    uint8_t     bitVal;

    switch(type) {
        case CODED_BLOCK_LUMA_DC:
        case CODED_BLOCK_LUMA_4x4:
            maxLevelCount = 16;
            break;

        case CODED_BLOCK_LUMA_AC:
        case CODED_BLOCK_CHROMA_AC:
            maxLevelCount = 15;
            break;

        case CODED_BLOCK_LUMA_8x8:
            maxLevelCount = 64;
            break;

        case CODED_BLOCK_CHROMA_DC:
            maxLevelCount = (chromaFormat == CHROMA_FORMAT_420) ? 4 : 8;
            break;

        default:
            maxLevelCount = 0;
    }
    noZeroLevelCount = maxLevelCount;

    ctxIdxOffsetForSCF   = ContextModelSet::getCtxIdxOffsetForSignificantCoeffFlag(ctxCompID, type);
    ctxIdxOffsetForLSCF  = ContextModelSet::getCtxIdxOffsetForLastSignificantCoeffFlag(ctxCompID, type);
    ctxIdxOffsetForCoeff = ContextModelSet::getCtxIdxOffsetForCoeffAbsLevelMinus1(ctxCompID, type);

    for(uint8_t idx = 0; idx < maxLevelCount - 1; ++idx) {
        ctxIdxInc = ContextModelSet::getCtxIdxIncForSignificantCoeffFlag(chromaFormat, type, idx);
        coefs[idx] = (int32_t)biariDecodeRegular(compID, ctxIdxOffsetForSCF + ctxIdxInc);

        if(coefs[idx]) {
            ctxIdxInc = ContextModelSet::getCtxIdxIncForLastSignificantCoeffFlag(chromaFormat, type, idx);
            bitVal = biariDecodeRegular(compID, ctxIdxOffsetForLSCF + ctxIdxInc);
            if(bitVal) {
                noZeroLevelCount = idx + 1;
                break;
            }      
        }
    }

    coefs[noZeroLevelCount - 1] = 1;
    ctxIdxInc = 1;

    for(int8_t idx = noZeroLevelCount - 1; idx >= 0; --idx) {
        if(coefs[idx] != 0)
            ctxIdxInc = readCoefLevel(compID, type, coefs + idx,
                ctxIdxOffsetForCoeff, ctxIdxInc);
    }
}

uint8_t CABAD::readCoefLevel(ColourComponent compID, CodedBlockType type, CoefType *coef, uint16_t ctxIdxOffset, uint8_t ctxIdxInc)
{
	uint32_t	codeNum = 0;
    uint16_t    ctxIdx;

    ctxIdx = ctxIdxOffset + (ctxIdxInc <= 4 ? ctxIdxInc : 0);
    if(biariDecodeRegular(compID, ctxIdx)) {
        codeNum += 1;
        ctxIdx = ctxIdxOffset + ((ctxIdxInc <= 4) ? 5 : ctxIdxInc);
        codeNum += biariDecodeTruncatedUnary(compID, 13, ctxIdx, ctxIdx);
        if(codeNum == 14)
           codeNum +=  biariDecodeUEG0(compID);
    }

    if(biariDecodeBypass(compID, 1U))
		*coef = (-1) * (int32_t)(codeNum +1);
	else
		*coef = (int32_t)(codeNum + 1);

    return ContextModelSet::updateCtxIdxIncForCoeffAbsLevelMinus1(type, ctxIdxInc, codeNum);
}

void CABAD::readEndOfSliceFlag(ColourComponent compID)
{
	uint8_t binVal;
	binVal = biariDecodeTerminate(compID);
}

void CABAD::updateCurFlags(ColourComponent planeID)
{
    if (planeID == COLOUR_COMPONENT_Y || (chromaFormat == CHROMA_FORMAT_444 && separateColourPlaneFlag)) {
        curFlags[planeID].nonIntraNxN = mb[planeID].mbPart != INTRA_PARTITION_4x4 &&
                                        mb[planeID].mbPart != INTRA_PARTITION_8x8;
        curFlags[planeID].mbQPDelta = mbQPDelta != 0;
        curFlags[planeID].intraChromaPredMode = mb[planeID].intraChromaPredMode != 0;
        curFlags[planeID].transform8x8 = mb[planeID].transformSize8x8Flag;
        curFlags[planeID].cbpLuma = cbpLuma[planeID];
        curFlags[planeID].cbpChroma = cbpChroma;
    }
    curFlags[planeID].nonZero = mb[planeID].nonZeroFlags; 
}