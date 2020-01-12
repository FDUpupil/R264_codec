#include "memory"

#include "decoder_top.h"
#include "parser.h"

DecoderTop::DecoderTop()
{
	nalu = new Bitstream[3];
}

DecoderTop::~DecoderTop()
{
	delete		core;
	delete		bsfile;
	delete[]	nalu;
}

void DecoderTop::decode(FILE *bs, FILE *frec)
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
				processSPS(sps, nalu[0]);
				displaySPS(sps);
				break;

			case NALU_TYPE_PPS:
				processPPS(pps, nalu[0]);
				displayPPS(pps);
				break;

			case NALU_TYPE_IDR:
				getSliceHeader(sps, pps, sliceInfo[0], nalu[0]);
				displaySliceHeader(sliceInfo[0]);

				if(sps.chroma_format_idc == CHROMA_FORMAT_444 && sps.separate_colour_plane_flag) {
					for(int compID = COLOUR_COMPONENT_CB; compID < COLOUR_COMPONENT_COUNT; ++compID)
						if(!bsfile->isEndofile() && bsfile->getNalu(nalu[compID])) {
							nalu[compID].getNalutype();
							getSliceHeader(sps, pps, sliceInfo[compID], nalu[compID]);
						} else 
							return;
				}

				setParConfig();
				decoderInit();

				core->decode(cfgSlic, recFrame, nalu);
			
				rec->require(move(buf));
				conv->convertMacroblocksToImage(*recFrame, *rec);
				buf = rec->release();
				fwrite(&buf[0], 1, rec->getFrameSize(), frec);

				break;
		}
		for (int i = 0; i < 3; i++)
			nalu[i].clear();
	}
}

void DecoderTop::setParConfig()
{
	/// set pic level config
	cfgPic.chromaFormat = sps.chroma_format_idc;
	cfgPic.bitDepthY = sps.bit_depth_luma_minus8 + 8;
	cfgPic.bitDepthC = sps.bit_depth_chroma_minus8 + 8;

	cfgPic.widthInMbs = sps.pic_width_in_mbs_minus1 + 1;
	cfgPic.heightInMbs = sps.pic_height_in_map_units_minus1 + 1;
	/// 
	switch(sps.chroma_format_idc) {
				case CHROMA_FORMAT_400:
				case CHROMA_FORMAT_444:
					cfgPic.cropInfo = {
							sps.frame_cropping_flag,
							sps.frame_crop_left_offset,
							sps.frame_crop_right_offset,
							sps.frame_crop_top_offset,
							sps.frame_crop_bottom_offset};
					break;
				case CHROMA_FORMAT_420:
					cfgPic.cropInfo = {
							sps.frame_cropping_flag,
							sps.frame_crop_left_offset * 2U,
							sps.frame_crop_right_offset * 2U,
							sps.frame_crop_top_offset * 2U,
							sps.frame_crop_bottom_offset * 2U };
					break;
				case CHROMA_FORMAT_422:
					cfgPic.cropInfo = {
							sps.frame_cropping_flag,
							sps.frame_crop_left_offset * 2U,
							sps.frame_crop_right_offset * 2U,
							sps.frame_crop_top_offset,
							sps.frame_crop_bottom_offset};
					break;
	}

	cfgPic.chromaQPIndexOffset = pps.chroma_qp_index_offset;
	cfgPic.secondChromaQPIndexOffset = pps.second_chroma_qp_index_offset;

	cfgPic.separateColourPlaneFlag = sps.separate_colour_plane_flag;
	cfgPic.transform8x8ModeFlag = pps.transform_8x8_mode_flag;
	cfgPic.entropyCodingModeFlag = pps.entropy_coding_mode_flag;
	cfgPic.deblockingFilteControlPresentFlag = pps.deblocking_filter_control_present_flag;

	cfgPic.scalingMatrix4x4Intra = &ScalingMatrixFlat4x4;
    cfgPic.scalingMatrix8x8Intra = &ScalingMatrixFlat8x8;
    cfgPic.scalingMatrix4x4Inter = &ScalingMatrixFlat4x4;
    cfgPic.scalingMatrix8x8Inter = &ScalingMatrixFlat8x8;

	/// set slice level config
	cfgSlic.sliceType = sliceInfo[COLOUR_COMPONENT_Y].slice_type;
	cfgSlic.idrPicId = (uint16_t)sliceInfo[COLOUR_COMPONENT_Y].pic_parameter_set_id;

	cfgSlic.firstMbInSlice = sliceInfo[COLOUR_COMPONENT_Y].first_mb_in_slice;

	cfgSlic.sliceQP[COLOUR_COMPONENT_Y] = pps.pic_init_qp_minus26 + 26 + sliceInfo[COLOUR_COMPONENT_Y].slice_qp_delta;
	cfgSlic.disableDeblockingFilterIdc[COLOUR_COMPONENT_Y] = sliceInfo[COLOUR_COMPONENT_Y].disable_deblocking_filter_idc;
	cfgSlic.sliceAlphaC0OffsetDiv2[COLOUR_COMPONENT_Y] = sliceInfo[COLOUR_COMPONENT_Y].slice_alpha_c0_offset_div2;
	cfgSlic.sliceBetaOffsetDiv2[COLOUR_COMPONENT_Y] = sliceInfo[COLOUR_COMPONENT_Y].slice_beta_offset_div2;

	for(uint8_t planeID = COLOUR_COMPONENT_CB; planeID < COLOUR_COMPONENT_COUNT; ++planeID)
		cfgSlic.sliceQP[planeID] = sps.separate_colour_plane_flag?  pps.pic_init_qp_minus26 + 26 + sliceInfo[planeID].slice_qp_delta : cfgSlic.sliceQP[COLOUR_COMPONENT_Y];


}

void DecoderTop::decoderInit()
{
	widthInMbs = cfgPic.widthInMbs;
	heightInMbs = cfgPic.heightInMbs;

	width = widthInMbs * 16 - cfgPic.cropInfo.leftOffset - cfgPic.cropInfo.rightOffset;
	height = heightInMbs * 16 - cfgPic.cropInfo.topOffset - cfgPic.cropInfo.bottomOffset;

	recFrame = std::make_unique<BlockyImage>(cfgPic.chromaFormat, widthInMbs, heightInMbs);
	rec = new RawImage(RAW_IMAGE_PLANAR_FORMAT, cfgPic.chromaFormat, cfgPic.bitDepthY, false, true, width, height);
	buf = std::make_unique<uint8_t[]>(rec->getFrameSize());
	conv = new MacroblockConverter(cfgPic.bitDepthY, cfgPic.cropInfo);
	core = new DecoderCore(cfgPic);

}