#ifndef __INTRA_BASE_H__
#define __INTRA_BASE_H__

#include "common/intra.h"
#include "common/cfgtype.h"
#include "decoder/tq/tq.h"



#include <cassert>

#define IntraBaseCheck(expr) assert(expr)

class IntraBase {
public:
    IntraBase(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg);
    virtual ~IntraBase() {}

    void init(const SliceLevelConfig &sliCfg);
    virtual void cycle(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbDec, const IntraToMemCtrl &memOut);

protected:
    Intra4x4Predictor *pred4x4[COLOUR_COMPONENT_COUNT];
    Intra8x8Predictor *pred8x8[COLOUR_COMPONENT_COUNT];
    Intra16x16Predictor *pred16x16[COLOUR_COMPONENT_COUNT];
    IntraChromaPredictor *predChroma[COLOUR_COMPONENT_COUNT - 1];

    TQ tq;

    // SPS/PPS level
    ChromaArrayType chromaFormat;
    uint8_t bitDepthY;
    uint8_t bitDepthC;

    uint8_t pic_init_qp;

    bool separateColourPlaneFlag;
    bool transform8x8ModeFlag;

    // Slice level
    SliceType sliceType;
    int8_t sliceQP;

    // Macroblock level
    uint16_t xInMbs;
    uint16_t yInMbs;
    int neighbour;
    int8_t mbQP[COLOUR_COMPONENT_COUNT];
    int8_t lastMbQP[COLOUR_COMPONENT_COUNT];

    uint8_t compCount;
    uint8_t planeCount;

    Blk<int, 4, 4> neighbour4x4;
    Blk<int, 2, 2> neighbour8x8;
    int neighbour16x16;
    int neighbourChroma;

    // Input
    const Macroblock *orgMb;
    Blk<PixType, 4, 4> orgMb4x4[COLOUR_COMPONENT_COUNT][4][4];
    Blk<PixType, 8, 8> orgMb8x8[COLOUR_COMPONENT_COUNT][2][2];
    const Blk<PixType, 16, 16> *orgMb16x16;
    const RefPixels *refPixels;
    const RefIntraModes *refModes;

    // Encode
    MacroblockPartition partition[COLOUR_COMPONENT_COUNT];
    // 4x4
    Blk<Intra4x4PredMode, 4, 4> predModes4x4[COLOUR_COMPONENT_COUNT];
    // 8x8
    Blk<Intra8x8PredMode, 2, 2> predModes8x8[COLOUR_COMPONENT_COUNT];
    // 16x16
    Intra16x16PredMode predMode16x16[COLOUR_COMPONENT_COUNT];
    // Chroma
    IntraChromaPredMode predModeChroma;

    // Output
    Macroblock *recMb;
    Blk<int, 4, 4> *predModes;
    EncodedMb *mbEnc;

    void start(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbEncoded, const IntraToMemCtrl &memOut);
    void finish(const MacroblockInfo &mbInfo, const MemCtrlToIntra &memIn, EncodedMb *mbEncoded, const IntraToMemCtrl &memOut);

    virtual void preprocess();
    virtual void estimate() = 0;
    virtual void decidePartition() = 0;
    virtual void encodeDecision();
    virtual void postprocess();

    int getNeighbourState4x4(uint8_t xInSbs, uint8_t yInSbs) const;
    int getNeighbourState8x8(uint8_t xInSbs, uint8_t yInSbs) const;
    int getNeighbourState16x16() const;
    int getNeighbourStateChroma() const;

    int getPredIntra4x4PredMode(ColourComponent planeID, uint8_t xInSbs, uint8_t yInSbs) const;
    int getPredIntra8x8PredMode(ColourComponent planeID, uint8_t xInSbs, uint8_t yInSbs) const;

    void setRefPixels4x4(ColourComponent planeID, const Blk<PixType, 16, 16> &recComp, uint8_t xInSbs, uint8_t yInSbs);
    void setRefPixels8x8(ColourComponent planeID, const Blk<PixType, 16, 16> &recComp, uint8_t xInSbs, uint8_t yInSbs);
    void setRefPixels16x16(ColourComponent planeID);
    void setRefPixelsChroma(ColourComponent planeID);

    void encodeDecision4x4(ColourComponent planeID);
    void encodeDecision8x8(ColourComponent planeID);
    void encodeDecision16x16(ColourComponent planeID);
    void encodeDecisionLuma(ColourComponent planeID);
    void encodeDecisionChroma();

    void encodeIntraModes4x4(ColourComponent compID);
    void encodeIntraModes8x8(ColourComponent compID);
    void encodeIntraModes16x16(ColourComponent compID);
    void encodeIntraModesChroma();

    virtual void encodeCoefs4x4(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);
    virtual void encodeCoefs8x8(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);
    virtual void encodeCoefs16x16(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);
    virtual void encodeCoefsChroma(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs);

    void encodePartition(ColourComponent compID);
    void encodeTransformSize8x8Flag(ColourComponent compID);
    void encodeNonZeroFlags(ColourComponent planeID);
    void encodeQP(ColourComponent compID);
};

inline int IntraBase::getNeighbourState8x8(uint8_t xInSbs, uint8_t yInSbs) const
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
    
    if (xInSbs == 0 && yInSbs == 0)
        nb |= (neighbour & NEIGHBOUR_AVAILABLE_UP) ? NEIGHBOUR_AVAILABLE_UP_RIGHT : 0;
    else if (xInSbs == 0)
        nb |= NEIGHBOUR_AVAILABLE_UP_RIGHT;
    else if (yInSbs == 0)
        nb |= neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT;

    return nb;
}

inline int IntraBase::getNeighbourState16x16() const
{
    return neighbour & ~NEIGHBOUR_AVAILABLE_UP_RIGHT;
}

inline int IntraBase::getNeighbourStateChroma() const
{
    return neighbour & ~NEIGHBOUR_AVAILABLE_UP_RIGHT;
}

inline int IntraBase::getPredIntra4x4PredMode(ColourComponent planeID, uint8_t xInSbs, uint8_t yInSbs) const
{
    int intra4x4PredModeA = !(neighbour4x4[yInSbs][xInSbs] & NEIGHBOUR_AVAILABLE_LEFT) ? INTRA_4x4_DC :
        (xInSbs == 0) ? refModes[planeID].l[yInSbs] : predModes4x4[planeID][yInSbs][xInSbs - 1];
    int intra4x4PredModeB = !(neighbour4x4[yInSbs][xInSbs] & NEIGHBOUR_AVAILABLE_UP) ? INTRA_4x4_DC :
        (yInSbs == 0) ? refModes[planeID].u[xInSbs] : predModes4x4[planeID][yInSbs - 1][xInSbs];

    if ((xInMbs == 0 && xInSbs == 0) || (yInMbs == 0 && yInSbs == 0))
        return INTRA_4x4_DC;
    else
        return Min(intra4x4PredModeA, intra4x4PredModeB);
}

inline int IntraBase::getPredIntra8x8PredMode(ColourComponent planeID, uint8_t xInSbs, uint8_t yInSbs) const
{
    int intra8x8PredModeA = !(neighbour8x8[yInSbs][xInSbs] & NEIGHBOUR_AVAILABLE_LEFT) ? INTRA_8x8_DC :
        (xInSbs == 0) ? refModes[planeID].l[yInSbs * 2] : predModes8x8[planeID][yInSbs][xInSbs - 1];
    int intra8x8PredModeB = !(neighbour8x8[yInSbs][xInSbs] & NEIGHBOUR_AVAILABLE_UP) ? INTRA_8x8_DC :
        (yInSbs == 0) ? refModes[planeID].u[xInSbs * 2] : predModes8x8[planeID][yInSbs - 1][xInSbs];

    if ((xInMbs == 0 && xInSbs == 0) || (yInMbs == 0 && yInSbs == 0))
        return INTRA_8x8_DC;
    else
        return Min(intra8x8PredModeA, intra8x8PredModeB);
}

#endif
