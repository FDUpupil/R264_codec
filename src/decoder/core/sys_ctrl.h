#ifndef _SYS_CTRL_H_
#define _SYS_CTRL_H_

#include "common/codec_type.h"

class SysCtrl {
public:
    SysCtrl(const PictureLevelConfig &cfgPic);

    void init(const SliceLevelConfig &cfgSlic);
    void getCurMbInfo(MacroblockInfo &mbInfo);
    void resetMbBuffer(EncodedMb *mbDec);
    void storeCurMbInfo(MacroblockInfo &mbInfo);
    bool hasNextMb() const;
    void setNextMb();
    void update(const EncodedMb *mbEnc);

private:
    uint16_t widthInMbs;
    uint16_t heightInMbs;

    uint16_t xInMbs;
    uint16_t yInMbs;

    int8_t curQP[COLOUR_COMPONENT_COUNT];
};

#endif
