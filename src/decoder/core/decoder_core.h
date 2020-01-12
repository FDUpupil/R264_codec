#ifndef _DECODER_CORE_H_
#define _DECODER_CORE_H_

#include "common/type.h"
#include "decoder/core/sys_ctrl.h"
#include "decoder/intra/intra_base.h"
#include "decoder/ec/cabad.h"
#include "decoder/db/deblock.h"
#include "tools/bitstream/bitstream.h"
#include "common/codec_type.h"
#include "mem_ctrl.h"


#include <memory>
#include <cassert>

#define DecoderCoreCheck(exper) assert(exper)

class DecoderCore {
public: 
    DecoderCore(const PictureLevelConfig &cfgPic);
    ~DecoderCore();

    void decode(const SliceLevelConfig &cfgSlic, std::unique_ptr<BlockyImage> &recFrame, Bitstream *sodb);

private:
    SysCtrl *sysCtrl;
    MemCtrl *memCtrl;
    IntraBase *rec;
    EntropyDecoder *ed;
    DeblockingFilter *db;

    MacroblockInfo mbInfo;
    MemCtrlToRec recIn;
    RecToMemCtrl recOut;
    MemCtrlToEC ecIn;
    ECToMemCtrl ecOut;
    MemCtrlToDb dbIn;
    DbToMemCtrl dbOut;

    EncodedMb mb[COLOUR_COMPONENT_COUNT];

    void runCycles();
};

#endif