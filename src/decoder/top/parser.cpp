#include<iostream>
#include"parser.h"

void processSPS(SequenceLevelConfig &seqcfg, Bitstream &nalu)
{
    if(nalu.bufempty())
        printf("error! nalu_buffer is empty!\n");
    else {
        seqcfg.profile_idc = (uint8_t)nalu.readFixedLength((uint8_t)8);

        seqcfg.constrained_set0_flag = nalu.readOneBit();
        seqcfg.constrained_set1_flag = nalu.readOneBit();
        seqcfg.constrained_set2_flag = nalu.readOneBit();
        seqcfg.constrained_set3_flag = nalu.readOneBit();
        seqcfg.constrained_set4_flag = nalu.readOneBit();
        seqcfg.constrained_set5_flag = nalu.readOneBit();

        //  reserved_zero_2bits
        nalu.readFixedLength((uint8_t)2);

        seqcfg.level_idc = (uint8_t)nalu.readFixedLength((uint8_t)8);
        seqcfg.seq_parameter_set_id = (uint8_t)nalu.readNextUEG0();
        seqcfg.chroma_format_idc = (ChromaArrayType)nalu.readNextUEG0();

        if(seqcfg.chroma_format_idc == CHROMA_FORMAT_444)
            seqcfg.separate_colour_plane_flag = nalu.readOneBit();

        seqcfg.bit_depth_luma_minus8 = (uint8_t)nalu.readNextUEG0();
        seqcfg.bit_depth_chroma_minus8 = (uint8_t)nalu.readNextUEG0();

        seqcfg.lossless_qpprime_flag = nalu.readOneBit();
        seqcfg.seq_scaling_matrix_present_flag = nalu.readOneBit();

        if(seqcfg.seq_scaling_matrix_present_flag)
            for(uint8_t idx = 0; idx < ((seqcfg.chroma_format_idc != CHROMA_FORMAT_444) ? 8 : 12); ++idx)
                seqcfg.seq_scaling_list_present_flag[idx] = nalu.readOneBit();

        seqcfg.log2_max_frame_num_minus4 = (uint8_t)nalu.readNextUEG0();
        seqcfg.pic_order_cnt_type = (uint8_t)nalu.readNextUEG0();

        seqcfg.max_num_ref_frames = (uint8_t)nalu.readNextUEG0();

        seqcfg.gaps_in_frame_num_value_allowed_flag = nalu.readOneBit();

        seqcfg.pic_width_in_mbs_minus1 = nalu.readNextUEG0();
        seqcfg.pic_height_in_map_units_minus1 = nalu.readNextUEG0();

        seqcfg.frame_mbs_only_flag = nalu.readOneBit();
        seqcfg.direct_8x8_inference_flag = nalu.readOneBit();
        seqcfg.frame_cropping_flag = nalu.readOneBit();

        if(seqcfg.frame_cropping_flag && (seqcfg.chroma_format_idc != CHROMA_FORMAT_400)){
            seqcfg.frame_crop_left_offset = (uint8_t)nalu.readNextUEG0();
            seqcfg.frame_crop_right_offset = (uint8_t)nalu.readNextUEG0();
            seqcfg.frame_crop_top_offset = (uint8_t)nalu.readNextUEG0();
            seqcfg.frame_crop_bottom_offset = (uint8_t)nalu.readNextUEG0();
        }

        seqcfg.vui_parameters_present_flag = nalu.readOneBit();

    }
}

void processPPS(PictureLevelConfig &piccfg, Bitstream &nalu)
{
    if(nalu.bufempty()) {
        printf("error! nalu_buffer is empty!\n");
    } else {
        piccfg.pic_parameter_set_id = (uint8_t)nalu.readNextUEG0();
        piccfg.seq_parameter_set_id = (uint8_t)nalu.readNextUEG0();
    
        piccfg.entropy_coding_mode_flag = nalu.readOneBit();
        piccfg.bottom_field_pic_order_in_frame_present_flag = nalu.readOneBit();

        piccfg.num_slice_groups_minus1 = nalu.readNextUEG0();

        piccfg.num_ref_idx_l0_default_active_minus1 = (uint8_t)nalu.readNextUEG0();
        piccfg.num_ref_idx_l1_default_active_minus1 = (uint8_t)nalu.readNextUEG0();

        piccfg.weighted_pred_flag = nalu.readOneBit();
        piccfg.weighted_bipred_idc = (uint8_t)nalu.readFixedLength((uint8_t)2);

        piccfg.pic_init_qp_minus26 = (int8_t)nalu.readNextSEG0();
        piccfg.pic_init_qs_minus26 = (int8_t)nalu.readNextSEG0();
        piccfg.chroma_qp_index_offset = (int8_t)nalu.readNextSEG0();

        piccfg.deblocking_filter_control_present_flag = nalu.readOneBit();
        piccfg.constrained_intra_pred_flag = nalu.readOneBit();
        piccfg.redundant_pic_cnt_present_flag = nalu.readOneBit();

		piccfg.transform_8x8_mode_flag = nalu.readOneBit();
		piccfg.pic_scaling_matrix_present_flag = nalu.readOneBit();

        piccfg.second_chroma_qp_index_offset = (int8_t)nalu.readNextSEG0();
    }
    
}

void getSliceHeader(SequenceLevelConfig &seqcfg, PictureLevelConfig &piccfg, SliceLevelConfig &sliCfg, Bitstream &nalu)
{
    if(nalu.bufempty()) {
        printf("error! nalu_buffer is empty!\n");
    } else {
        sliCfg.first_mb_in_slice = nalu.readNextUEG0();
        sliCfg.slice_type = (SliceType)nalu.readNextUEG0();

        sliCfg.pic_parameter_set_id = nalu.readNextUEG0();

        if(seqcfg.chroma_format_idc == CHROMA_FORMAT_444)
        sliCfg.colour_plane_id = (ColourComponent)nalu.readFixedLength((uint8_t)2);

        sliCfg.frame_num = nalu.readFixedLength(4);

        if(!seqcfg.frame_mbs_only_flag){
            sliCfg.field_pic_flag = nalu.readOneBit();
            if(sliCfg.field_pic_flag)
                sliCfg.bottom_field_flag = nalu.readOneBit();
        }
    
        if(sliCfg.slice_type == I_SLICE){
            sliCfg.idr_pic_id = (uint8_t)nalu.readNextUEG0();
            sliCfg.no_output_of_prior_pics_flag = nalu.readOneBit();
            sliCfg.long_term_reference_flag = nalu.readOneBit();
        }

        sliCfg.slice_qp_delta = (int8_t)nalu.readNextSEG0();

	    if (piccfg.deblocking_filter_control_present_flag) {
		    sliCfg.disable_deblocking_filter_idc = (uint8_t)nalu.readNextUEG0();
		    if (sliCfg.disable_deblocking_filter_idc != 1) {
			    sliCfg.slice_alpha_c0_offset_div2 = (int8_t)nalu.readNextSEG0();
			    sliCfg.slice_beta_offset_div2 = (int8_t)nalu.readNextSEG0();
		    }
	    }
    }  
	nalu.align();
}

void displaySPS(SequenceLevelConfig &seqcfg)
{
    printf("profile_idc:\t%d\n", seqcfg.profile_idc);
    printf("level_idc:\t%d\n", seqcfg.level_idc);
    printf("seq_parameter_set_id:\t%d\n", seqcfg.seq_parameter_set_id);
    printf("chroma_format_idc:\t%d\n", (uint8_t)seqcfg.chroma_format_idc);
    if(seqcfg.chroma_format_idc == CHROMA_FORMAT_444)
        printf("separate_colour_plane_flag:\t%s\n", seqcfg.separate_colour_plane_flag ? "TRUE" : "FLASE");
    printf("bit_depth_luma_minus8:\t%d\n", seqcfg.bit_depth_luma_minus8);
    printf("bit_depth_chroma_minus8:\t%d\n", seqcfg.bit_depth_chroma_minus8);
    printf("lossless_qpprime_flag:\t%s\n", seqcfg.lossless_qpprime_flag ? "TRUE" : "FLASE");
    printf("seq_scaling_matrix_present_flag:\t%s\n", seqcfg.seq_scaling_matrix_present_flag ? "TRUE" : "FLASE");

    printf("log2_max_frame_num_minus4:\t%d\n", seqcfg.log2_max_frame_num_minus4);
    printf("pic_order_cnt_type:\t%d\n", seqcfg.pic_order_cnt_type);

    printf("max_num_ref_frames:\t%d\n", seqcfg.max_num_ref_frames);
    printf("gaps_in_frame_num_value_allowed_flag:\t%s\n", seqcfg.gaps_in_frame_num_value_allowed_flag ? "TRUE" : "FLASE");
    printf("pic_width_in_mbs_minus1:\t%d\n", seqcfg.pic_width_in_mbs_minus1);
    printf("pic_height_in_map_units_minus1:\t%d\n", seqcfg.pic_height_in_map_units_minus1);
    printf("frame_mbs_only_flag:\t%s\n", seqcfg.frame_mbs_only_flag ? "TRUE" : "FLASE");

    printf("direct_8x8_inference_flag:\t%s\n", seqcfg.direct_8x8_inference_flag ? "TRUE" : "FLASE");
    printf("frame_cropping_flag:\t%s\n", seqcfg.frame_cropping_flag ? "TRUE" : "FLASE");

    if(seqcfg.frame_cropping_flag && (seqcfg.chroma_format_idc != CHROMA_FORMAT_400)){
        printf("frame_crop_left_offset:\t%d\n", seqcfg.frame_crop_left_offset);
        printf("frame_crop_right_offset:\t%d\n", seqcfg.frame_crop_right_offset);
        printf("frame_crop_top_offset:\t%d\n", seqcfg.frame_crop_top_offset);
        printf("frame_crop_bottom_offset:\t%d\n", seqcfg.frame_crop_bottom_offset);
    }

    printf("vui_parameters_present_flag:\t%s\n", seqcfg.vui_parameters_present_flag ? "TRUE" : "FLASE");

}

void displayPPS(PictureLevelConfig &piccfg)
{
    printf("pic_parameter_set_id:\t%d\n", piccfg.pic_parameter_set_id);
    printf("seq_parameter_set_id:\t%d\n", piccfg.seq_parameter_set_id);

    printf("entropy_coding_mode_flag:\t%s\n", piccfg.entropy_coding_mode_flag ? "TRUE" : "FLASE");
    printf("bottom_field_pic_order_in_frame_present_flag:\t%s\n", piccfg.bottom_field_pic_order_in_frame_present_flag ? "TRUE" : "FLASE");

    printf("num_slice_groups_minus1:\t%d\n", piccfg.num_slice_groups_minus1);

    printf("num_ref_idx_l0_default_active_minus1:\t%d\n", piccfg.num_ref_idx_l0_default_active_minus1);
    printf("num_ref_idx_l1_default_active_minus1:\t%d\n", piccfg.num_ref_idx_l1_default_active_minus1);

    printf("weighted_pred_flag:\t%s\n", piccfg.weighted_pred_flag ? "TRUE" : "FLASE");
    printf("weighted_bipred_idc:\t%d\n", piccfg.weighted_bipred_idc);

    printf("pic_init_qp_minus26:\t%d\n", piccfg.pic_init_qp_minus26);
    printf("pic_init_qs_minus26:\t%d\n", piccfg.pic_init_qs_minus26);

    printf("chroma_qp_index_offset:\t%d\n", piccfg.chroma_qp_index_offset);

    printf("deblocking_filter_control_present_flag:\t%s\n", piccfg.deblocking_filter_control_present_flag ? "TRUE" : "FLASE");
    printf("constrained_intra_pred_flag:\t%s\n", piccfg.constrained_intra_pred_flag ? "TRUE" : "FLASE");
    printf("redundant_pic_cnt_present_flag:\t%s\n", piccfg.redundant_pic_cnt_present_flag ? "TRUE" : "FLASE");

    printf("second_chroma_qp_index_offset:\t%d\n", piccfg.second_chroma_qp_index_offset);
}

void displaySliceHeader(SliceLevelConfig &sliCfg)
{
	printf("first_mb_in_slice:\t%d\n", sliCfg.first_mb_in_slice);
	printf("slice_type:\t%d\n", (uint8_t)sliCfg.slice_type);
	printf("pic_parameter_set_id:\t%d\n", sliCfg.pic_parameter_set_id);

	printf("frame_num:\t%d\n", sliCfg.frame_num);

	printf("idr_pic_id:\t%d\n", sliCfg.idr_pic_id);

	printf("no_output_of_prior_pics_flag:\t%s\n", sliCfg.no_output_of_prior_pics_flag ? "TRUE" : "FLASE");
	printf("long_term_reference_flag:\t%s\n", sliCfg.long_term_reference_flag ? "TRUE" : "FLASE");

	printf("slice_qp_delta:\t%d\n", sliCfg.slice_qp_delta);

	printf("disable_deblocking_filter_idc:\t%d\n", sliCfg.disable_deblocking_filter_idc);
}

void mbInfoDisplay(EncodedMb* mbEnc)
{
	uint8_t subMbCount=0;
	switch (mbEnc->mbPart) {
		case INTRA_PARTITION_4x4:
			subMbCount = 16;
			printf("partition: INTRA_PARTITION_4x4\n");
			break;
		case INTRA_PARTITION_8x8:
			subMbCount = 4;
			printf("partition: INTRA_PARTITION_8x8\n");
			break;
		case INTRA_PARTITION_16x16:
			subMbCount = 1;
			printf("partition: INTRA_PARTITION_16x16\n");
			break;
	}

	printf("transformSize8x8Flag:\t%s\n", mbEnc->transformSize8x8Flag ? "TRUE" : "FLASE");

	if (subMbCount == 1) {
		printf("16x16preMode:%d\n", mbEnc->intra16x16PredMode);
	}
	else {
		for (int i = 0; i < subMbCount; i++) {
			printf("subMb [%d] : prev:%d", i, mbEnc->prevIntraPredModeFlag[i]);
			if (!mbEnc->prevIntraPredModeFlag[i])
				printf("preMode : %d", mbEnc->remIntraPredMode[i]);
			printf("\n");
		}
	}

	//printf("ChromaPreMode : %d\n", mbEnc->intraChromaPredMode);

	printf("QP : %d\n", mbEnc->qp);

}