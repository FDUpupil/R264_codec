#ifndef _CFGTYPE_H_
#define _CFGTYPE_H_

#define MAXnum_slice_groups_minus1 8

#include"type.h"

const CostType MAX_COST = ~0U;

struct SequenceLevelConfig {

    uint8_t             profile_idc;                                          // u(8)
    bool                constrained_set0_flag;                                // u(1)
    bool                constrained_set1_flag;                                // u(1)
    bool                constrained_set2_flag;                                // u(1)
    bool                constrained_set3_flag;                                // u(1)
    bool                constrained_set4_flag;                                // u(1)
    bool                constrained_set5_flag;                                // u(1)

    // reserved_zero_2bits

    uint8_t             level_idc;                                            // u(8)
    uint8_t             seq_parameter_set_id;                                 // ue(v) (0~31)
    ChromaArrayType     chroma_format_idc;                                    // ue(V) (0~3)

    // if chroma_format_idc=yuv444
    bool                separate_colour_plane_flag;                           // u(1)

    uint8_t             bit_depth_luma_minus8;                                // ue(v)
    uint8_t             bit_depth_chroma_minus8;                              // ue(v)
    bool                lossless_qpprime_flag;                                // u(1)
    bool                seq_scaling_matrix_present_flag;                      // u(1)

    // if seq_scaling_matrix_present_flag
    bool                seq_scaling_list_present_flag[12];                    // u(1)
    
    uint8_t             log2_max_frame_num_minus4;                            // ue(v) (0~12)
    uint8_t             pic_order_cnt_type;                                   // ue(v) (0~2)

    // if pic_order_cnt_type == 0
    uint8_t             log2_max_pic_order_cnt_lsb_minus4;                    // ue(v)

    // if pic_order_cnt_type == 1
    bool                delta_pic_order_always_zero_flag;                     // u(1)
    int8_t              offset_for_non_ref_pic;                               // se(v)
    int8_t              offset_for_top_to_bottom_field;                       // se(v)
    uint8_t             num_ref_frames_in_pic_order_cnt_cycle;                // ue(v)
    int8_t              offset_for_ref_frame[256];                            // se(v)

    // if pic_order_cnt_type == 2 default

    uint8_t             max_num_ref_frames;                                   // ue(v) (0~MaxDpbFrames)
    bool                gaps_in_frame_num_value_allowed_flag;                 // u(1)
    uint32_t            pic_width_in_mbs_minus1;                              // ue(v)
    uint32_t            pic_height_in_map_units_minus1;                       // ue(v)
    bool                frame_mbs_only_flag;                                  // u(1)

    // if !frame_mbs_only_flag
    bool                mb_adaptive_frame_field_flag;                         // u(1)

    bool                direct_8x8_inference_flag;                            // u(1)
    bool                frame_cropping_flag;                                  // u(1)

    // if frame_cropping_flag
    uint8_t             frame_crop_left_offset;                               // ue(v)
    uint8_t             frame_crop_right_offset;                              // ue(v)
    uint8_t             frame_crop_top_offset;                                // ue(v)
    uint8_t             frame_crop_bottom_offset;                             // ue(v)

    bool                vui_parameters_present_flag;                          // u(1)

};

struct PictureLevelConfig {

    uint8_t     pic_parameter_set_id;                                 // ue(v) (0~255)
    uint8_t     seq_parameter_set_id;                                 // ue(v)
    bool        entropy_coding_mode_flag;                             // u(1)
    bool        bottom_field_pic_order_in_frame_present_flag;         // u(1)
    uint8_t     num_slice_groups_minus1;                              // ue(v)

    // if num_slice_groups_minus1>0
    uint8_t     slice_group_map_type;                                 // ue(v)
    uint8_t     run_length_minus1[MAXnum_slice_groups_minus1] ;       // ue(v)
    uint8_t     top_left[MAXnum_slice_groups_minus1];                 // ue(v)
    uint8_t     bottom_right[MAXnum_slice_groups_minus1];             // ue(v)
    // if slice_group_map_type == 3/4/5
    bool        slice_group_change_direction_flag;                    // u(1)
    uint8_t     slice_group_change_rate_minus1;                       // ue(v)
    // if slice_group_map_type == 6
    uint8_t     pic_size_in_map_units_minus1;                         // ue(v)
    uint8_t     *slice_group_id;                                      // u(v) v =  NumberBitsPerSliceGroupId

    uint8_t     num_ref_idx_l0_default_active_minus1;                 // ue(v) (0~31)
    uint8_t     num_ref_idx_l1_default_active_minus1;                 // ue(v) (0~31)
    bool        weighted_pred_flag;                                   // u(1)
    uint8_t     weighted_bipred_idc;                                  // u(2)
    int8_t      pic_init_qp_minus26;                                  // se(v)
    int8_t      pic_init_qs_minus26;                                  // se(v) (-12 ~ 12)
    int8_t      chroma_qp_index_offset;                               // se(v)
    bool        deblocking_filter_control_present_flag;               // u(1)
    bool        constrained_intra_pred_flag;                          // u(1)
    bool        redundant_pic_cnt_present_flag;                       // u(1)

    // if more rbsp data
    bool        transform_8x8_mode_flag;                              // u(1)
    bool        pic_scaling_matrix_present_flag;                      // u(1)
    bool        pic_scaling_list_present_flag[12];                    // u(1)
    
    const Blk<WeightScaleType, 4, 4> *scalingMatrix4x4Intra;
    const Blk<WeightScaleType, 8, 8> *scalingMatrix8x8Intra;
    const Blk<WeightScaleType, 4, 4> *scalingMatrix4x4Inter;
    const Blk<WeightScaleType, 8, 8> *scalingMatrix8x8Inter;

    int8_t      second_chroma_qp_index_offset;                        // se(v) (-12 ~ 12)
};

struct SliceLevelConfig {
    uint32_t    first_mb_in_slice;                                    // ue(v)
    SliceType   slice_type;                                           // ue(v)

    uint32_t    pic_parameter_set_id;                                 // ue(v)

    //if separate_colour_plane_flag == 1
    ColourComponent     colour_plane_id;                                      // u(2)

    uint32_t    frame_num;                                            // ue(v)

    // if !frame_mbs_only_flag
    bool        field_pic_flag;                                       // u(1)
    // if field_pic_flag
    bool        bottom_field_flag;                                    // u(1)

    // if slice_type == I_SLICE
    uint8_t     idr_pic_id;                                           // ue(v)
    bool        no_output_of_prior_pics_flag;                         // u(1)
    bool        long_term_reference_flag;                             // u(1)

    int8_t      slice_qp_delta;                                       // se(v)

    // if deblocking_filter_control_present_flag
    uint8_t     disable_deblocking_filter_idc;                        // ue(v) (0~2)

    int8_t      slice_alpha_c0_offset_div2;                           // se(v) (-6~6)
    int8_t      slice_beta_offset_div2;                               // se(v) (-6~6)

    uint8_t     cabacInitIdc;
};

struct EncodedMb {
    MacroblockPartition mbPart;
    uint8_t transformSize8x8Flag;

    // Intra
    uint8_t prevIntraPredModeFlag[16];
    uint8_t remIntraPredMode[16];
    uint8_t intra16x16PredMode;
    uint8_t intraChromaPredMode;

    // Common
    CompCoefs coefs;
    uint32_t nonZeroFlags;
    int8_t qp;
    bool qpUpdated;
};

struct MacroblockInfo {
    // Intrinsic
    uint16_t xInMbs;
    uint16_t yInMbs;
    int neighbour;

    // Rate control
    int8_t mbQP[COLOUR_COMPONENT_COUNT];
};

struct MemCtrlToIntra {
    const Macroblock *orgMb;
    const RefPixels *refPixels;
    const RefIntraModes *refModes;
};

struct IntraToMemCtrl {
    Macroblock *recMb;
    Blk<int, 4, 4> *predModes;
};

struct MemCtrlToEC {
    CAVLCRefCounts *cavlcRefCounts;
    CABACRefFlags *cabacRefFlags;
};

struct ECToMemCtrl {
    CAVLCCurCounts *cavlcCurCounts;
    CABACCurFlags *cabacCurFlags;
};

struct MemCtrlToDb {
	const Macroblock* curMb;
	const Macroblock* leftMb;
	const Macroblock* upMb;
	DbRefQP* refQP;
};

struct DbToMemCtrl {
	Macroblock* curMb;
	Macroblock* leftMb;
	Macroblock* upMb;
	DbCurQP* curQP;
};

#endif