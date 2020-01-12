#ifndef __DEBLOCK_H__
#define __DEBLOCK_H__

#include "common/codec_type.h"
#include "common/type.h"

class DeblockingFilter {
public:
    DeblockingFilter(const PictureLevelConfig &cfg);

    void init(const SliceLevelConfig &cfg);
    void cycle(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut);

private:
    ChromaArrayType chromaFormat;
    uint8_t bitDepthY;
    uint8_t bitDepthC;
    int8_t cbQPIndexOffset;
    int8_t crQPIndexOffset;
    bool separateColourPlaneFlag;

    uint8_t compCount;
    uint8_t planeCount;

    uint8_t disableDeblockingFilterIdc[COLOUR_COMPONENT_COUNT];
    int8_t filterOffsetA[COLOUR_COMPONENT_COUNT];
    int8_t filterOffsetB[COLOUR_COMPONENT_COUNT];

    uint16_t xInMbs;
    uint16_t yInMbs;
    int neighbour;
    
    const EncodedMb *mb;

    Macroblock *curMb;
    Macroblock *leftMb;
    Macroblock *upMb;
    DbRefQP *refQP;
    DbCurQP *curQP;

    uint8_t bSHor[COLOUR_COMPONENT_COUNT][4][4];
    uint8_t bSVer[COLOUR_COMPONENT_COUNT][4][4];

    static const uint8_t alphaMap[52];
    static const uint8_t betaMap[52];
    static const uint8_t tc0Map[3][52];

    void start(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut);
    void finish(const MacroblockInfo &mbInfo, const EncodedMb *mbEnc, const MemCtrlToDb &memIn, const DbToMemCtrl &memOut);
    void preprocess();
    void postprocess();

    void setQP();
    void deriveStrength(ColourComponent compID);
    void filterLumaBlk(ColourComponent planeID);
    void filterChromaBlk(ColourComponent planeID);
    void filterLumaEdge(
        ColourComponent planeID, uint8_t indexA, uint8_t indexB, uint8_t bS,
        PixType &p3, PixType &p2, PixType &p1, PixType &p0,
        PixType &q0, PixType &q1, PixType &q2, PixType &q3) const;
    void filterChromaEdge(
        uint8_t indexA, uint8_t indexB, uint8_t bS,
        PixType &p3, PixType &p2, PixType &p1, PixType &p0,
        PixType &q0, PixType &q1, PixType &q2, PixType &q3) const;
};

#endif
