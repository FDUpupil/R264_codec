#include "decoder_core.h"
#include "decoder/intra/intra_normal.h"

#include "decoder/top/parser.h"

#define ORG_BS R"(org_bs.txt)"

DecoderCore::DecoderCore(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg)
{
    sysCtrl = new SysCtrl(seqCfg, picCfg);
    memCtrl = new MemCtrl(seqCfg, picCfg);
    //intra   = new IntraNormal(seqCfg, picCfg);

    if(picCfg.entropy_coding_mode_flag)
       ed = new CABAD(seqCfg, picCfg);

    //db reserve
}

DecoderCore::~DecoderCore()
{
    delete sysCtrl;
    delete memCtrl;

    delete intra;
    delete ed;

}

void DecoderCore::decode(const SliceLevelConfig &sliCfg, std::unique_ptr<BlockyImage> &recFrame, Bitstream *sodb)
{
    sysCtrl->init(sliCfg);
    memCtrl->init(sliCfg);
    //intra->init(sliCfg);
    ed->init(sliCfg, sodb);

    runCycles();
}

void DecoderCore::runCycles()
{
	int mb_num = 0;

    while(sysCtrl->hasNextMb()) {
        sysCtrl->getNextMbInfo(mbInfo); // set mb Spatial information & update next mb
		sysCtrl->prepareNextMb(mb);	// reset mb encoding information 
		memCtrl->getECMemory(ecIn, ecOut); // set ref&cur flag ptr
						
	    ed->cycle(mbInfo, mb, ecIn, ecOut);

        memCtrl->updateECMemory(); //update the ref flag 
		memCtrl->setNextMb(); // update the spatial information in memctrl

		mbInfoDisplay(mb);

		FILE* forg_bs;
		forg_bs = fopen(ORG_BS, "a");
		fprintf(forg_bs, "\n ========== %d mb finished! \n", mb_num);
		printf("\n ========== %d mb finished! \n", mb_num);
		mb_num++;
		fclose(forg_bs);
    }
}
