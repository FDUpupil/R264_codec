#ifndef _SYS_CTRL_H_
#define _SYS_CTRL_H_

#include "common/cfgtype.h"

class SysCtrl {
public:
    SysCtrl(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg);

    void init(const SliceLevelConfig &slicCfg);
    void getNextMbInfo(MacroblockInfo &mbInfo);
    void prepareNextMb(EncodedMb *mbEnc);
    void storeCurMbInfo(MacroblockInfo &mbInfo);
    bool hasNextMb() const;
    void update(const EncodedMb *mbEnc);

private:
    uint16_t widthInMbs;
    uint16_t heightInMbs;

    uint16_t xInMbs;
    uint16_t yInMbs;

    int8_t curQP[COLOUR_COMPONENT_COUNT];
};

#endif
