#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include "common/parset_common.h"
#include "common/codec_type.h"
#include "tools/bitstream/bitstream.h"

void processSPS(SequenceParameterSet &sps, Bitstream &nalu);
void processPPS(PictureParameterSet &pps, Bitstream &nalu);
void getSliceHeader(SequenceParameterSet &sps, PictureParameterSet &pps, SliceHeader &sliceInfo, Bitstream &nalu);

void displaySPS(SequenceParameterSet &sps);
void displayPPS(PictureParameterSet &pps);
void displaySliceHeader(SliceHeader &sliceInfo);
void mbInfoDisplay(EncodedMb* mbEnc);

#endif