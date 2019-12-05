#ifndef _CABAD_H_
#define _CABAD_H_

#include "entropy_decoder.h"
#include "arith_dec_engine.h"
#include "common/cabac.h"
#include "common/type.h"

#define NON_ZERO_FLAG_DC_SHIFT 16

class CABAD : public EntropyDecoder {
public:
    CABAD(const PictureLevelConfig &cfgPic);
    ~CABAD();

    virtual void init(const SliceLevelConfig &cfgSlic, Bitstream *bs);

private:
    ContextModelSet *ctxModel;
    ArithDecEngine *engine;

    CABACRefFlags *refFlags;
    CABACCurFlags *curFlags;
    
    virtual void start(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut);
    virtual void finish(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut);

    virtual void preprocess();
	virtual void postprocess();

	virtual void readSliceDataInfos(ColourComponent compID);

    virtual void readMbTypeInfos(ColourComponent compID);
    virtual void readIntraNxNPredModeInfos(ColourComponent compID);
    virtual void readIntraChromaPredMode();
    virtual void readTransformSize8x8FlagInfos(ColourComponent compID);
    virtual void readCodedBlockPatternInfos(ColourComponent compID);
    virtual void readMbQPDeltaInfos(ColourComponent compID);
    virtual void readLumaCoefsInfos(ColourComponent compID, ColourComponent planeID);
    virtual void readChromaCoefsInfos();
	virtual void readEndOfSliceFlag(ColourComponent compID);

    void readCodedBlockPatternLuma(ColourComponent compID);
    void readCodedBlockPatternChroma();
    void readBlkCoefs(ColourComponent compID, ColourComponent ctxID, CodedBlockType type, CoefType *coefs);
    uint8_t readCoefLevel(ColourComponent compID, CodedBlockType type, CoefType *coef, uint16_t ctxIdxOffset, uint8_t ctxIdxInc);

    void updateCurFlags(ColourComponent planeID);

    CordTermFlag2D<1> getFlagForCodedBlockFlagLumaDC(ColourComponent planeID);
    CordTermFlag2D<1> getFlagForCodedBlockFlagLumaAC(ColourComponent planeID, uint8_t idx);
    CordTermFlag2D<1> getFlagForCodedBlockFlagLuma4x4(ColourComponent planeID, uint8_t idx);
    CordTermFlag2D<1> getFlagForCodedBlockFlagLuma8x8(ColourComponent planeID, uint8_t idx);
    CordTermFlag2D<1> getFlagForCodedBlockFlagChromaDC(ColourComponent planeID);
    CordTermFlag2D<1> getFlagForCodedBlockFlagChromaAC(ColourComponent planeID, uint8_t idx);

    uint8_t biariDecodeRegular(ColourComponent compID, uint16_t ctxIdx);
    uint32_t biariDecodeBypass(ColourComponent compID, uint8_t length);
    uint8_t biariDecodeTerminate(ColourComponent compID);

    uint32_t biariDecodeFixedlLength(ColourComponent compID, uint8_t length, uint16_t ctxIdx);
    uint8_t biariDecodeTruncatedUnary(ColourComponent compID, uint8_t bitMax, uint16_t ctxIdx, uint16_t ctxIdxMax);
    uint8_t biariDecodeUnary(ColourComponent compID, uint16_t ctxIdx, uint16_t ctxIdxMax);
    uint32_t biariDecodeUEG0(ColourComponent compID);
};

#endif