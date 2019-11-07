#include "memory"

#include "decoder_top.h"
#include "parser.h"

DecoderTop::DecoderTop()
{
	nalu = new Bitstream[3];
}

DecoderTop::~DecoderTop()
{
	delete[] core;
	delete[] bsfile;
	delete[] nalu;
}

void DecoderTop::decode(FILE* bs)
{

	//init bitstream
	bsfile = new Annex_b(bs);

	//get NALU package
	while (!bsfile->isEndofile()) {
		if (bsfile->getNalu(nalu[0]) == 0)
			return;
		NALUType nalu_type = nalu[0].getNalutype(); /// get NALU type
		switch (nalu_type) {
		case NALU_TYPE_SPS:
			processSPS(seqCfg, nalu[0]);
			displaySPS(seqCfg);
			recFrame = std::make_unique<BlockyImage>(seqCfg.chroma_format_idc, (uint16_t)seqCfg.pic_width_in_mbs_minus1 + 1, (uint16_t)seqCfg.pic_height_in_map_units_minus1 + 1);
			break;

		case NALU_TYPE_PPS:
			processPPS(picCfg, nalu[0]);
			displayPPS(picCfg);

			core = new DecoderCore(seqCfg, picCfg);
			break;

		case NALU_TYPE_IDR:
			getSliceHeader(seqCfg, picCfg, sliCfg[0], nalu[0]);
			displaySliceHeader(sliCfg[0]);

			if(seqCfg.chroma_format_idc == CHROMA_FORMAT_444 && seqCfg.separate_colour_plane_flag) {
				for(int compID = 1; compID < COLOUR_COMPONENT_COUNT; ++compID)
					if(!bsfile->isEndofile() && bsfile->getNalu(nalu[compID])) {
						nalu[compID].getNalutype();
						getSliceHeader(seqCfg, picCfg, sliCfg[compID], nalu[compID]);
					} else 
						return;
			}
			core->decode(sliCfg[0], recFrame, nalu);

			break;
		}
		for (int i = 0; i < 3; i++)
			nalu[i].clear();
	}
}