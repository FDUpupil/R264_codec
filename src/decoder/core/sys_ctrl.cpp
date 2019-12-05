#include "sys_ctrl.h"

SysCtrl::SysCtrl(const PictureLevelConfig &cfgPic)
    : widthInMbs(cfgPic.widthInMbs),
      heightInMbs(cfgPic.heightInMbs)
{
}

void SysCtrl::init(const SliceLevelConfig &cfgSlic)
{

    xInMbs = 0;
    yInMbs = 0;

    for(uint8_t planeID = COLOUR_COMPONENT_Y; planeID < COLOUR_COMPONENT_COUNT; ++planeID)
        curQP[planeID] = cfgSlic.sliceQP[planeID];

}

void SysCtrl::getCurMbInfo(MacroblockInfo &mbInfo)
{
    int nb = 0;

    if (xInMbs != 0)
        nb |= NEIGHBOUR_AVAILABLE_LEFT;
    
    if (xInMbs != 0 && yInMbs != 0)
        nb |= NEIGHBOUR_AVAILABLE_UP_LEFT;

    if (yInMbs != 0)
        nb |= NEIGHBOUR_AVAILABLE_UP;

    if (xInMbs != widthInMbs - 1 && yInMbs != 0)
        nb |= NEIGHBOUR_AVAILABLE_UP_RIGHT;

    mbInfo.xInMbs = xInMbs;
    mbInfo.yInMbs = yInMbs;
    mbInfo.neighbour = nb;
    mbInfo.mbQP[COLOUR_COMPONENT_Y]  = curQP[COLOUR_COMPONENT_Y];
    // mbInfo.mbQP[COLOUR_COMPONENT_CB] = curQP[COLOUR_COMPONENT_CB];
    // mbInfo.mbQP[COLOUR_COMPONENT_CR] = curQP[COLOUR_COMPONENT_CR];

    // if (++xInMbs == widthInMbs) {
    //     xInMbs = 0;
    //     ++yInMbs;
    // }
}

void SysCtrl::resetMbBuffer(EncodedMb *mbEnc)
{
    for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
    mbEnc[planeID].nonZeroFlags = 0;
    for(uint8_t yIn4x4 = 0; yIn4x4 < 4; ++yIn4x4) {
        for(uint8_t xIn4x4 = 0; xIn4x4 < 4; ++xIn4x4) {
            mbEnc[planeID].coefs.blk16x16.dc[yIn4x4 * 4 + xIn4x4] = 0;
            for(uint8_t idx = 0; idx < 16; ++idx) {
                mbEnc[planeID].coefs.blk4x4[yIn4x4 * 4 + xIn4x4][idx] = 0;
                if(idx >= 1)
                    mbEnc[planeID].coefs.blk16x16.ac[yIn4x4 * 4 + xIn4x4][idx - 1] = 0;
            }
        }
    }
    for(uint8_t idxIn8x8 = 0; idxIn8x8 < 4; ++idxIn8x8) {
        for(uint8_t idx = 0; idx < 64; ++idx)
            mbEnc[planeID].coefs.blk8x8[idxIn8x8][idx] = 0;
    }
    }
}

void SysCtrl::setNextMb()
{
    if (++xInMbs == widthInMbs) { 
        xInMbs = 0;
        ++yInMbs;
    }
}

bool SysCtrl::hasNextMb() const
{
    return yInMbs < heightInMbs;
}

void SysCtrl::update(const EncodedMb *mbEnc)
{
    Unused(mbEnc);
}