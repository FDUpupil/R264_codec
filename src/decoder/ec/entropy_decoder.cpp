#include"entropy_decoder.h"

EntropyDecoder::EntropyDecoder(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg)
    : chromaFormat(seqCfg.chroma_format_idc),
      separateColourPlaneFlag(seqCfg.separate_colour_plane_flag),
      transform8x8ModeFlag(picCfg.transform_8x8_mode_flag)
{
    bitDepthY = seqCfg.bit_depth_luma_minus8 + 8;
    bitDepthC = seqCfg.bit_depth_chroma_minus8 + 8;

    widthInMbs = seqCfg.pic_width_in_mbs_minus1 + 1;
    heightInMbs = seqCfg.pic_height_in_map_units_minus1 + 1;

    pic_init_qp = picCfg.pic_init_qp_minus26 + 26;

    compCount = (uint8_t)((chromaFormat == CHROMA_FORMAT_444 && separateColourPlaneFlag) ? COLOUR_COMPONENT_COUNT : 1);
    planeCount = (uint8_t)((chromaFormat != CHROMA_FORMAT_400) ? COLOUR_COMPONENT_COUNT : 1);
    
}

EntropyDecoder::~EntropyDecoder()
{
}


void EntropyDecoder::init(const SliceLevelConfig &sliCfg)
{
    sliceType = sliCfg.slice_type;
    sliceQP = (uint8_t)(pic_init_qp + sliCfg.slice_qp_delta);
    //currentPlaneID = (chromaFormat == CHROMA_FORMAT_444 && separateColourPlaneFlag )? sliCfg.colour_plane_id : COLOUR_COMPONENT_Y;
}

void EntropyDecoder::cycle(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut)
{
    start(mbInfo, mbDec, memIn, memOut);

    preprocess();
    /// read every NALU package
    for(int compID = 0; compID < compCount; ++compID)
        readSliceDataInfos((ColourComponent)compID);
    postprocess();

    finish(mbInfo, mbDec, memIn, memOut);
}

void EntropyDecoder::start(MacroblockInfo &mbInfo, EncodedMb *mbDec, MemCtrlToEC &memIn, ECToMemCtrl &memOut)
{
    // MacroblockInfo
    xInMbs = mbInfo.xInMbs;
    yInMbs = mbInfo.yInMbs;

	// EncodedMb
	mb = mbDec;
}

void EntropyDecoder::finish(MacroblockInfo& mbInfo, EncodedMb* mbDec, MemCtrlToEC& memIn, ECToMemCtrl& memOut)
{
}

void EntropyDecoder::preprocess()
{
    for(int compID = 0; compID < compCount; ++compID)
        cbpLuma[compID] = 0;
    cbpChroma = 0;
}

void EntropyDecoder::postprocess()
{
}

void EntropyDecoder::readSliceDataInfos(ColourComponent compID)
{
    readMbTypeInfos(compID);

    if((mb[compID].mbPart != INTRA_PARTITION_16x16) && transform8x8ModeFlag)
        readTransformSize8x8FlagInfos(compID);

    if(mb[compID].mbPart != INTRA_PARTITION_16x16)
        readIntraNxNPredModeInfos(compID);
    
    if (chromaFormat == CHROMA_FORMAT_420 || chromaFormat == CHROMA_FORMAT_422)
        readIntraChromaPredMode();

    if (mb[compID].mbPart != INTRA_PARTITION_16x16)
        readCodedBlockPatternInfos(compID);

    if(cbpLuma[compID] ||
        ((chromaFormat == CHROMA_FORMAT_420 || chromaFormat == CHROMA_FORMAT_422) && cbpChroma) ||
        mb[compID].mbPart == INTRA_PARTITION_16x16) {
            readMbQPDeltaInfos(compID);
            readCoefs(compID);
        }
}

void EntropyDecoder::readCoefs(ColourComponent compID)
{
    readLumaCoefsInfos(compID, compID);
    if(chromaFormat != CHROMA_FORMAT_400) {
        if(chromaFormat != CHROMA_FORMAT_444) {
            readChromaCoefsInfos();
        }
        else if(!separateColourPlaneFlag) {
            readLumaCoefsInfos(compID, COLOUR_COMPONENT_CB);
            readLumaCoefsInfos(compID, COLOUR_COMPONENT_CR);
        }
    }
}