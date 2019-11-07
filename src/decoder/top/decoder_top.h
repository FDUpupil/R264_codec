#ifndef _DECODER_TOP_H_
#define _DECODER_TOP_H_

#include "memory"

#include "common/cfgtype.h"
#include "common/type.h"
#include "tools/bitstream/annex_b.h"
#include "decoder/core/decoder_core.h"

class DecoderTop {
public:
    DecoderTop();
    ~DecoderTop();

    void decode(FILE* bs);

private:
    uint8_t     compCount;
    uint8_t     planeCount;
    uint8_t     chromaRate;

    std::unique_ptr<BlockyImage>  recFrame;

    DecoderCore *core;
    Annex_b     *bsfile;
    Bitstream   *nalu;

    SequenceLevelConfig     seqCfg;
    PictureLevelConfig      picCfg;
    SliceLevelConfig        sliCfg[3];

};

#endif