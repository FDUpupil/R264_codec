#include "decoder_core.h"
#include "decoder/intra/intra_rec.h"

#include "decoder/top/parser.h"

#define ORG_BS R"(org_bs.txt)"

DecoderCore::DecoderCore(const PictureLevelConfig &cfgPic)
{
    sysCtrl = new SysCtrl(cfgPic);
    memCtrl = new MemCtrl(cfgPic);
    rec     = new IntraRec(cfgPic);

    //if(picCfg.entropy_coding_mode_flag)
    ed      = new CABAD(cfgPic);

    //db reserve
    db      = new DeblockingFilter(cfgPic);
}

DecoderCore::~DecoderCore()
{
    delete sysCtrl;
    delete memCtrl;

    delete rec;
    delete ed;
    delete db;

}

void DecoderCore::decode(const SliceLevelConfig &cfgSlic, std::unique_ptr<BlockyImage> &recFrame, Bitstream *sodb)
{
    sysCtrl->init(cfgSlic);
    memCtrl->init(cfgSlic);
    rec->init(cfgSlic);
    ed->init(cfgSlic, sodb);
    db->init(cfgSlic);
    memCtrl->requireRecFrame(std::move(recFrame));

    runCycles();

    recFrame = memCtrl->releaseRecFrame();
}

void DecoderCore::runCycles()
{
	int mb_num = 0;

    while(sysCtrl->hasNextMb()) {
        sysCtrl->getCurMbInfo(mbInfo); // set mb Spatial information & update next mb
		sysCtrl->resetMbBuffer(mb);	// reset mb encoding information

        memCtrl->getRecMemory(recIn, recOut); // set ref-pixel and ref-predMode
		memCtrl->getECMemory(ecIn, ecOut); // set ref&cur flag ptr
		memCtrl->getDbMemory(dbIn,dbOut); // 

	    ed->cycle(mbInfo, mb, ecIn, ecOut);
        memCtrl->updateECMemory(); //update the ref flag 

        //mbInfoDisplay(mb);

        rec->cycle(mbInfo, recIn, mb, recOut);
        memCtrl->updateIntraMemory();

        //db process
        db->cycle(mbInfo, mb, dbIn, dbOut);
        memCtrl->updateDbMemory();

        sysCtrl->setNextMb();
		memCtrl->setNextMb(); // update the spatial information in memctrl

		

		//FILE* forg_bs;
		//forg_bs = fopen(ORG_BS, "a");
		//fprintf(forg_bs, "\n ========== %d mb finished! \n", mb_num);
		//printf("\n ========== %d mb finished! \n", mb_num);
		//mb_num++;
		//fclose(forg_bs);
    }
}
