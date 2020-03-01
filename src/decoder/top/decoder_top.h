#ifndef _DECODER_TOP_H_
#define _DECODER_TOP_H_

#include "memory"

#include "common/parset_common.h"
#include "common/codec_type.h"
#include "common/type.h"
#include "tools/bitstream/annex_b.h"
#include "tools/image/raw_image.h"
#include "tools/image/macroblock_converter.h"
#include "decoder/core/decoder_core.h"

class DecoderTop {
public:
    DecoderTop();
    ~DecoderTop();

    void decode(FILE *bs, FILE *frec);

private:
    uint8_t     compCount;
    uint8_t     planeCount;
    uint8_t     chromaRate;

    uint16_t    width;
    uint16_t    height;
    uint16_t    widthInMbs;
    uint16_t    heightInMbs;

    std::unique_ptr<BlockyImage>            recFrame;
    std::unique_ptr<uint8_t[]>              buf;
    std::unique_ptr<RawImage>               rec;
    std::unique_ptr<MacroblockConverter>    conv;

    std::unique_ptr<DecoderCore>            core;
    
    Annex_b     *bsfile;
    Bitstream   *nalu;

    SequenceParameterSet    sps;
    PictureParameterSet     pps;
    SliceHeader             sliceInfo[COLOUR_COMPONENT_COUNT];

    PictureLevelConfig      cfgPic;
    SliceLevelConfig        cfgSlic;

    void setPicParConfig();
    void setSlcParConfig();
    void decoderInit();
};

#endif