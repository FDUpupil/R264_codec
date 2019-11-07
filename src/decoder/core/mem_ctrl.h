#ifndef __MEM_CTRL_H__
#define __MEM_CTRL_H__

#include"common/cfgtype.h"
#include"common/block.h"
#include"common/type.h"
#include"tools/image/blocky_image.h"

#include<memory>
#include<cassert>

#define MemCtrlCheck(expr) assert(expr)

class MemCtrl {
public:
    MemCtrl(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg);
    ~MemCtrl();

    void init(const SliceLevelConfig &sliCfg);
    void cycle();
    void requireOrgFrame(std::unique_ptr<BlockyImage> _orgFrame);
    void requireRecFrame(std::unique_ptr<BlockyImage> _recFrame);
    std::unique_ptr<BlockyImage> releaseOrgFrame();
    std::unique_ptr<BlockyImage> releaseRecFrame();

    void getIntraMemory(MemCtrlToIntra &memIn, IntraToMemCtrl &memOut);
    void getECMemory(MemCtrlToEC &memIn, ECToMemCtrl &memOut);
    void getDbMemory(MemCtrlToDb &memIn, DbToMemCtrl &memOut);
    void updateIntraMemory();
    void updateECMemory();
    void updateDbMemory();
    void setNextMb();

private:
    ChromaArrayType chromaFormat;
    uint16_t widthInMbs;
    uint16_t heightInMbs;
    bool separateColourPlaneFlag;
    bool entropyCodingModeFlag;

    uint8_t compCount;
    uint8_t planeCount;

    uint16_t xInMbs;
    uint16_t yInMbs;

    std::unique_ptr<BlockyImage> orgFrame;
    std::unique_ptr<BlockyImage> recFrame;

    Macroblock *curOrgMb;
    Macroblock *curRecMb;

    // Intra
    RefPixels curRefPixels[COLOUR_COMPONENT_COUNT];
    RefIntraModes curRefIntraModes[COLOUR_COMPONENT_COUNT];
    Blk<int, 4, 4> curIntraPredModes[COLOUR_COMPONENT_COUNT];

    // CAVLC
    CAVLCRefCounts refCounts[COLOUR_COMPONENT_COUNT];
    CAVLCCurCounts curCounts[COLOUR_COMPONENT_COUNT];

    uint8_t *leftNonZeroCount[COLOUR_COMPONENT_COUNT];
    uint8_t *upNonZeroCount[COLOUR_COMPONENT_COUNT];

    // CABAC
    CABACRefFlags refFlags[COLOUR_COMPONENT_COUNT];
    CABACCurFlags curFlags[COLOUR_COMPONENT_COUNT];

    // Deblocking Filter
    DbRefQP refQP[COLOUR_COMPONENT_COUNT];
    DbCurQP curQP[COLOUR_COMPONENT_COUNT];

    PixType *lastHorLinePixels[COLOUR_COMPONENT_COUNT];
    PixType *lastVerLinePixels[COLOUR_COMPONENT_COUNT];
    PixType *lastUpLeftPixels[COLOUR_COMPONENT_COUNT];
    uint8_t *lastHorLineIntraPredModes[COLOUR_COMPONENT_COUNT];
    uint8_t *lastVerLineIntraPredModes[COLOUR_COMPONENT_COUNT];

    uint8_t *prevMbQPDelta[COLOUR_COMPONENT_COUNT];
    uint8_t *leftNonIntraNxN[COLOUR_COMPONENT_COUNT];
    uint8_t *upNonIntraNxN[COLOUR_COMPONENT_COUNT];
    uint8_t *leftIntraChromaPredMode[COLOUR_COMPONENT_COUNT];
    uint8_t *upIntraChromaPredMode[COLOUR_COMPONENT_COUNT];
    uint8_t *leftTransform8x8[COLOUR_COMPONENT_COUNT];
    uint8_t *upTransform8x8[COLOUR_COMPONENT_COUNT];
    uint8_t *leftCBPLuma[COLOUR_COMPONENT_COUNT];
    uint8_t *upCBPLuma[COLOUR_COMPONENT_COUNT];
    uint8_t *leftCBPChroma[COLOUR_COMPONENT_COUNT];
    uint8_t *upCBPChroma[COLOUR_COMPONENT_COUNT];
    uint8_t *leftNonZero[COLOUR_COMPONENT_COUNT];
    uint8_t *upNonZero[COLOUR_COMPONENT_COUNT];
    uint8_t *leftNonZeroDC[COLOUR_COMPONENT_COUNT];
    uint8_t *upNonZeroDC[COLOUR_COMPONENT_COUNT];

    int8_t *leftQP[COLOUR_COMPONENT_COUNT];
    int8_t *upQP[COLOUR_COMPONENT_COUNT];

    void getIntraPixels();
    void getIntraPredModes();
    void getCAVLCRefCounts();
    void getCABACRefFlags();
    void getDbRefQP();
    void updateIntraPixels();
    void updateIntraPredModes();
    void updateCAVLCMemory();
    void updateCABACMemory();
};

#endif
