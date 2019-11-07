#ifndef _DECODER_CORE_H_
#define _DECODER_CORE_H_

#include "common/type.h"
#include "decoder/core/sys_ctrl.h"
#include "decoder/intra/intra_base.h"
#include "decoder/ec/cabad.h"
#include "tools/bitstream/bitstream.h"
#include "common/cfgtype.h"
#include "mem_ctrl.h"


#include <memory>
#include <cassert>

#define DecoderCoreCheck(exper) assert(exper)

class DecoderCore {
public: 
    DecoderCore(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg);
    ~DecoderCore();

    void decode(const SliceLevelConfig &sliCfg, std::unique_ptr<BlockyImage> &recFrame, Bitstream *sodb);

private:
    SysCtrl *sysCtrl;
    MemCtrl *memCtrl;
    IntraBase *intra;
    EntropyDecoder *ed;

    MacroblockInfo mbInfo;
    MemCtrlToIntra intraIn;
    IntraToMemCtrl intraOut;
    MemCtrlToEC ecIn;
    ECToMemCtrl ecOut;
    EncodedMb mb[COLOUR_COMPONENT_COUNT];

    void runCycles();
};

#endif