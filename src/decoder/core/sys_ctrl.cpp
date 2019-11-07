#include "sys_ctrl.h"

SysCtrl::SysCtrl(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg)
    : widthInMbs(seqCfg.pic_width_in_mbs_minus1 + 1),
      heightInMbs(seqCfg.pic_height_in_map_units_minus1 + 1)
{
}

void SysCtrl::init(const SliceLevelConfig &sliCfg)
{
    Unused(sliCfg);

    xInMbs = 0;
    yInMbs = 0;

    curQP[COLOUR_COMPONENT_Y] += sliCfg.slice_qp_delta;

}

void SysCtrl::getNextMbInfo(MacroblockInfo &mbInfo)
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

    if (++xInMbs == widthInMbs) {
        xInMbs = 0;
        ++yInMbs;
    }
}

void SysCtrl::prepareNextMb(EncodedMb *mbEnc)
{
    for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
    mbEnc[planeID].nonZeroFlags = 0;
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