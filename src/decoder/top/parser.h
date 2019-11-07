#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>
#include"common/cfgtype.h"
#include"tools/bitstream/bitstream.h"

void processSPS(SequenceLevelConfig &seqcfg, Bitstream &nalu);
void processPPS(PictureLevelConfig &piccfg, Bitstream &nalu);
void getSliceHeader(SequenceLevelConfig &seqcfg, PictureLevelConfig &piccfg, SliceLevelConfig &cfgSlice, Bitstream &nalu);

void displaySPS(SequenceLevelConfig &seqcfg);
void displayPPS(PictureLevelConfig &piccfg);
void displaySliceHeader(SliceLevelConfig &cfgSlice);
void mbInfoDisplay(EncodedMb* mbEnc);

#endif