#ifndef _ENTROPY_DECODER_H_
#define _ENTROPY_DECODER_H_

#include<memory>
#include "common/codec_type.h"
#include "common/type.h"
#include "tools/bitstream/bitstream.h"

class EntropyDecoder {
public:
    EntropyDecoder(const PictureLevelConfig &cfgPic);
    virtual ~EntropyDecoder();

    virtual void init(const SliceLevelConfig &sliCfg);
    virtual void init(const SliceLevelConfig &cfgSlic, Bitstream *rbsp) = 0;
    virtual void cycle(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut);

protected:
    // SPS level
    ChromaArrayType chromaFormat;
    uint8_t bitDepthY;
    uint8_t bitDepthC;

    uint16_t widthInMbs;
    uint16_t heightInMbs;

    bool separateColourPlaneFlag;
    
    // PPS level
    uint8_t pic_init_qp;

    bool transform8x8ModeFlag;

    // Slice level
    SliceType sliceType;
    int8_t sliceQP[COLOUR_COMPONENT_COUNT];
    //ColourComponent currentPlaneID;

    // Macroblock level
    uint16_t xInMbs;
    uint16_t yInMbs;

    //int8_t mbQP;
    int8_t lastMbQP[COLOUR_COMPONENT_COUNT];
    int8_t mbQPDelta;

    uint8_t compCount; ///< NALU package number
    uint8_t planeCount; ///< the picture luma&chroma plane number 

    EncodedMb *mb;   
    // CodedBlockPattern
    uint8_t cbpLumaDCFlag; // used for 16x16 partition
    uint8_t cbpLumaACFlag; // used for 16x16 partition

    uint8_t cbpLuma[COLOUR_COMPONENT_COUNT];
    uint8_t cbpChroma;
    
    virtual void start(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut);
    virtual void finish(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut);
    virtual void preprocess();
    virtual void postprocess();

    virtual void readSliceDataInfos(ColourComponent compID);

    virtual void readMbTypeInfos(ColourComponent compID) = 0;
    virtual void readIntraNxNPredModeInfos(ColourComponent compID) = 0;
    virtual void readIntraChromaPredMode() = 0;
    virtual void readTransformSize8x8FlagInfos(ColourComponent compID) = 0;
    virtual void readCodedBlockPatternInfos(ColourComponent compID) = 0;
    virtual void readMbQPDeltaInfos(ColourComponent compID) = 0;
    virtual void readCoefs(ColourComponent compID);
    virtual void readLumaCoefsInfos(ColourComponent compID, ColourComponent planeID) = 0;
    virtual void readChromaCoefsInfos() = 0;
	virtual void readEndOfSliceFlag(ColourComponent compID) = 0;

};

#endif