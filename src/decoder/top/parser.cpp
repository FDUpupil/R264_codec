#include<iostream>
#include "parser.h"
#include "common/quant_common.h"

void processSPS(SequenceParameterSet &sps, Bitstream &nalu)
{
    if(nalu.bufempty())
        printf("error! nalu_buffer is empty!\n");
    else {
        sps.profile_idc = (uint8_t)nalu.readFixedLength((uint8_t)8);

        sps.constrained_set0_flag = nalu.readOneBit();
        sps.constrained_set1_flag = nalu.readOneBit();
        sps.constrained_set2_flag = nalu.readOneBit();
        sps.constrained_set3_flag = nalu.readOneBit();
        sps.constrained_set4_flag = nalu.readOneBit();
        sps.constrained_set5_flag = nalu.readOneBit();

        //  reserved_zero_2bits
        nalu.readFixedLength((uint8_t)2);

        sps.level_idc = (uint8_t)nalu.readFixedLength((uint8_t)8);
        sps.seq_parameter_set_id = (uint8_t)nalu.readNextUEG0();
        sps.chroma_format_idc = (ChromaArrayType)nalu.readNextUEG0();

        if(sps.chroma_format_idc == CHROMA_FORMAT_444)
            sps.separate_colour_plane_flag = nalu.readOneBit();
        else
            sps.separate_colour_plane_flag = false;

        sps.bit_depth_luma_minus8 = (uint8_t)nalu.readNextUEG0();
        sps.bit_depth_chroma_minus8 = (uint8_t)nalu.readNextUEG0();

        sps.lossless_qpprime_flag = nalu.readOneBit();
        sps.seq_scaling_matrix_present_flag = nalu.readOneBit();

        if(sps.seq_scaling_matrix_present_flag)
            for(uint8_t idx = 0; idx < ((sps.chroma_format_idc != CHROMA_FORMAT_444) ? 8 : 12); ++idx)
                sps.seq_scaling_list_present_flag[idx] = nalu.readOneBit();

        sps.log2_max_frame_num_minus4 = (uint8_t)nalu.readNextUEG0();
        sps.pic_order_cnt_type = (uint8_t)nalu.readNextUEG0();

        sps.max_num_ref_frames = (uint8_t)nalu.readNextUEG0();

        sps.gaps_in_frame_num_value_allowed_flag = nalu.readOneBit();

        sps.pic_width_in_mbs_minus1 = nalu.readNextUEG0();
        sps.pic_height_in_map_units_minus1 = nalu.readNextUEG0();

        sps.frame_mbs_only_flag = nalu.readOneBit();
        sps.direct_8x8_inference_flag = nalu.readOneBit();
        sps.frame_cropping_flag = nalu.readOneBit();

        if(sps.frame_cropping_flag) {
            sps.frame_crop_left_offset = (uint8_t)nalu.readNextUEG0();
            sps.frame_crop_right_offset = (uint8_t)nalu.readNextUEG0();
            sps.frame_crop_top_offset = (uint8_t)nalu.readNextUEG0();
            sps.frame_crop_bottom_offset = (uint8_t)nalu.readNextUEG0();
        }

        sps.vui_parameters_present_flag = nalu.readOneBit();

    }
}

void processPPS(PictureParameterSet &pps, Bitstream &nalu)
{
    if(nalu.bufempty()) {
        printf("error! nalu_buffer is empty!\n");
    } else {
        pps.pic_parameter_set_id = (uint8_t)nalu.readNextUEG0();
        pps.seq_parameter_set_id = (uint8_t)nalu.readNextUEG0();
    
        pps.entropy_coding_mode_flag = nalu.readOneBit();
        pps.bottom_field_pic_order_in_frame_present_flag = nalu.readOneBit();

        pps.num_slice_groups_minus1 = nalu.readNextUEG0();

        pps.num_ref_idx_l0_default_active_minus1 = (uint8_t)nalu.readNextUEG0();
        pps.num_ref_idx_l1_default_active_minus1 = (uint8_t)nalu.readNextUEG0();

        pps.weighted_pred_flag = nalu.readOneBit();
        pps.weighted_bipred_idc = (uint8_t)nalu.readFixedLength((uint8_t)2);

        pps.pic_init_qp_minus26 = (int8_t)nalu.readNextSEG0();
        pps.pic_init_qs_minus26 = (int8_t)nalu.readNextSEG0();
        pps.chroma_qp_index_offset = (int8_t)nalu.readNextSEG0();

        pps.deblocking_filter_control_present_flag = nalu.readOneBit();
        pps.constrained_intra_pred_flag = nalu.readOneBit();
        pps.redundant_pic_cnt_present_flag = nalu.readOneBit();

		pps.transform_8x8_mode_flag = nalu.readOneBit();
		pps.pic_scaling_matrix_present_flag = nalu.readOneBit();

        pps.second_chroma_qp_index_offset = (int8_t)nalu.readNextSEG0();

        pps.scalingMatrix4x4Intra = &ScalingMatrixFlat4x4;
        pps.scalingMatrix8x8Intra = &ScalingMatrixFlat8x8;
        pps.scalingMatrix4x4Inter = &ScalingMatrixFlat4x4;
        pps.scalingMatrix8x8Inter = &ScalingMatrixFlat8x8;
    }
    
}

void getSliceHeader(SequenceParameterSet &sps, PictureParameterSet &pps, SliceHeader &sliceheader, Bitstream &nalu)
{
    if(nalu.bufempty()) {
        printf("error! nalu_buffer is empty!\n");
    } else {
        sliceheader.first_mb_in_slice = nalu.readNextUEG0();
        sliceheader.slice_type = (SliceType)nalu.readNextUEG0();

        sliceheader.pic_parameter_set_id = nalu.readNextUEG0();

        if(sps.chroma_format_idc == CHROMA_FORMAT_444 & sps.separate_colour_plane_flag)
        sliceheader.colour_plane_id = (ColourComponent)nalu.readFixedLength((uint8_t)2);

        sliceheader.frame_num = nalu.readFixedLength(4);

        if(!sps.frame_mbs_only_flag){
            sliceheader.field_pic_flag = nalu.readOneBit();
            if(sliceheader.field_pic_flag)
                sliceheader.bottom_field_flag = nalu.readOneBit();
        }
    
        if(sliceheader.slice_type == I_SLICE){
            sliceheader.idr_pic_id = (uint8_t)nalu.readNextUEG0();
            sliceheader.no_output_of_prior_pics_flag = nalu.readOneBit();
            sliceheader.long_term_reference_flag = nalu.readOneBit();
        }

        sliceheader.slice_qp_delta = (int8_t)nalu.readNextSEG0();

	    if (pps.deblocking_filter_control_present_flag) {
		    sliceheader.disable_deblocking_filter_idc = (uint8_t)nalu.readNextUEG0();
		    if (sliceheader.disable_deblocking_filter_idc != 1) {
			    sliceheader.slice_alpha_c0_offset_div2 = (int8_t)nalu.readNextSEG0();
			    sliceheader.slice_beta_offset_div2 = (int8_t)nalu.readNextSEG0();
		    }
	    }
    }  
	nalu.align_weak();
}

void displaySPS(SequenceParameterSet &sps)
{
    printf("profile_idc:\t%d\n", sps.profile_idc);
    printf("level_idc:\t%d\n", sps.level_idc);
    printf("seq_parameter_set_id:\t%d\n", sps.seq_parameter_set_id);
    printf("chroma_format_idc:\t%d\n", (uint8_t)sps.chroma_format_idc);
    if(sps.chroma_format_idc == CHROMA_FORMAT_444)
        printf("separate_colour_plane_flag:\t%s\n", sps.separate_colour_plane_flag ? "TRUE" : "FLASE");
    printf("bit_depth_luma_minus8:\t%d\n", sps.bit_depth_luma_minus8);
    printf("bit_depth_chroma_minus8:\t%d\n", sps.bit_depth_chroma_minus8);
    printf("lossless_qpprime_flag:\t%s\n", sps.lossless_qpprime_flag ? "TRUE" : "FLASE");
    printf("seq_scaling_matrix_present_flag:\t%s\n", sps.seq_scaling_matrix_present_flag ? "TRUE" : "FLASE");

    printf("log2_max_frame_num_minus4:\t%d\n", sps.log2_max_frame_num_minus4);
    printf("pic_order_cnt_type:\t%d\n", sps.pic_order_cnt_type);

    printf("max_num_ref_frames:\t%d\n", sps.max_num_ref_frames);
    printf("gaps_in_frame_num_value_allowed_flag:\t%s\n", sps.gaps_in_frame_num_value_allowed_flag ? "TRUE" : "FLASE");
    printf("pic_width_in_mbs_minus1:\t%d\n", sps.pic_width_in_mbs_minus1);
    printf("pic_height_in_map_units_minus1:\t%d\n", sps.pic_height_in_map_units_minus1);
    printf("frame_mbs_only_flag:\t%s\n", sps.frame_mbs_only_flag ? "TRUE" : "FLASE");

    printf("direct_8x8_inference_flag:\t%s\n", sps.direct_8x8_inference_flag ? "TRUE" : "FLASE");
    printf("frame_cropping_flag:\t%s\n", sps.frame_cropping_flag ? "TRUE" : "FLASE");

    if(sps.frame_cropping_flag && (sps.chroma_format_idc != CHROMA_FORMAT_400)){
        printf("frame_crop_left_offset:\t%d\n", sps.frame_crop_left_offset);
        printf("frame_crop_right_offset:\t%d\n", sps.frame_crop_right_offset);
        printf("frame_crop_top_offset:\t%d\n", sps.frame_crop_top_offset);
        printf("frame_crop_bottom_offset:\t%d\n", sps.frame_crop_bottom_offset);
    }

    printf("vui_parameters_present_flag:\t%s\n", sps.vui_parameters_present_flag ? "TRUE" : "FLASE");

}

void displayPPS(PictureParameterSet &pps)
{
    printf("pic_parameter_set_id:\t%d\n", pps.pic_parameter_set_id);
    printf("seq_parameter_set_id:\t%d\n", pps.seq_parameter_set_id);

    printf("entropy_coding_mode_flag:\t%s\n", pps.entropy_coding_mode_flag ? "TRUE" : "FLASE");
    printf("bottom_field_pic_order_in_frame_present_flag:\t%s\n", pps.bottom_field_pic_order_in_frame_present_flag ? "TRUE" : "FLASE");

    printf("num_slice_groups_minus1:\t%d\n", pps.num_slice_groups_minus1);

    printf("num_ref_idx_l0_default_active_minus1:\t%d\n", pps.num_ref_idx_l0_default_active_minus1);
    printf("num_ref_idx_l1_default_active_minus1:\t%d\n", pps.num_ref_idx_l1_default_active_minus1);

    printf("weighted_pred_flag:\t%s\n", pps.weighted_pred_flag ? "TRUE" : "FLASE");
    printf("weighted_bipred_idc:\t%d\n", pps.weighted_bipred_idc);

    printf("pic_init_qp_minus26:\t%d\n", pps.pic_init_qp_minus26);
    printf("pic_init_qs_minus26:\t%d\n", pps.pic_init_qs_minus26);

    printf("chroma_qp_index_offset:\t%d\n", pps.chroma_qp_index_offset);

    printf("deblocking_filter_control_present_flag:\t%s\n", pps.deblocking_filter_control_present_flag ? "TRUE" : "FLASE");
    printf("constrained_intra_pred_flag:\t%s\n", pps.constrained_intra_pred_flag ? "TRUE" : "FLASE");
    printf("redundant_pic_cnt_present_flag:\t%s\n", pps.redundant_pic_cnt_present_flag ? "TRUE" : "FLASE");

    printf("second_chroma_qp_index_offset:\t%d\n", pps.second_chroma_qp_index_offset);
}

void displaySliceHeader(SliceHeader &sliceInfo)
{
	printf("first_mb_in_slice:\t%d\n", sliceInfo.first_mb_in_slice);
	printf("slice_type:\t%d\n", (uint8_t)sliceInfo.slice_type);
	printf("pic_parameter_set_id:\t%d\n", sliceInfo.pic_parameter_set_id);

	printf("frame_num:\t%d\n", sliceInfo.frame_num);

	printf("idr_pic_id:\t%d\n", sliceInfo.idr_pic_id);

	printf("no_output_of_prior_pics_flag:\t%s\n", sliceInfo.no_output_of_prior_pics_flag ? "TRUE" : "FLASE");
	printf("long_term_reference_flag:\t%s\n", sliceInfo.long_term_reference_flag ? "TRUE" : "FLASE");

	printf("slice_qp_delta:\t%d\n", sliceInfo.slice_qp_delta);

	printf("disable_deblocking_filter_idc:\t%d\n", sliceInfo.disable_deblocking_filter_idc);
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