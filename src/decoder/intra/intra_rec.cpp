#include "intra_rec.h"


IntraRec::IntraRec(const PictureLevelConfig &cfg) 
    :  IntraBase(cfg)
{
}

void IntraRec::reconstructure()
{
    if(chromaFormat == CHROMA_FORMAT_444) {
        for(int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
            switch(mbEnc[planeID].mbPart) {
                case INTRA_PARTITION_4x4:
                    reconstructure4x4((ColourComponent)planeID);
                    break;
                
                case INTRA_PARTITION_8x8:
                    reconstructure8x8((ColourComponent)planeID);
                    break;

                case INTRA_PARTITION_16x16:
                    reconstructure16x16((ColourComponent)planeID);
                    break;
            }
        }
    } else {
        switch(mbEnc[COLOUR_COMPONENT_Y].mbPart) {
                case INTRA_PARTITION_4x4:
                    reconstructure4x4(COLOUR_COMPONENT_Y);
                    break;
                
                case INTRA_PARTITION_8x8:
                    reconstructure8x8(COLOUR_COMPONENT_Y);
                    break;

                case INTRA_PARTITION_16x16:
                    reconstructure16x16(COLOUR_COMPONENT_Y);
                    break;
        }
        if(chromaFormat != CHROMA_FORMAT_400)
            reconstructureChroma();
    }
}

void IntraRec::reconstructure4x4(ColourComponent planeID)
{
    Intra4x4Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> recResBlk;
    uint8_t bitDepth = (planeID == COLOUR_COMPONENT_Y) ? bitDepthY : bitDepthC;

    for(uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;

        setRefPixels4x4(planeID, recMb4x4[planeID], xInSbs, yInSbs);
        
        //printf("%d submb predict! \n", idx);
        pred4x4[planeID]->predict(predModes4x4[planeID][yInSbs][xInSbs], predBlk);

        tq.inverse4x4(planeID, mbEnc[planeID].coefs.blk4x4[idx], recResBlk);

        for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
            for (uint8_t xOfSbs = 0; xOfSbs < 4; ++xOfSbs)
                (*recMb)[planeID][yInSbs * 4 + yOfSbs][xInSbs * 4 + xOfSbs] =
                    recMb4x4[planeID][yInSbs * 4 + yOfSbs][xInSbs * 4 + xOfSbs] =
                    (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
    }

}

void IntraRec::reconstructure8x8(ColourComponent planeID)
{
    Intra8x8Predictor::PredBlk predBlk;
    Blk<ResType, 8, 8> recResBlk;
    uint8_t bitDepth = (planeID == COLOUR_COMPONENT_Y) ? bitDepthY : bitDepthC;

    for(uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;

        setRefPixels8x8(planeID, recMb8x8[planeID], xInSbs, yInSbs);

        //printf("%d submb predict! \n", idx);
        pred8x8[planeID]->predict(predModes8x8[planeID][yInSbs][xInSbs], predBlk);
       
        tq.inverse8x8(planeID, mbEnc[planeID].coefs.blk8x8[idx], recResBlk);

        for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
            for (uint8_t xOfSbs = 0; xOfSbs < 8; ++xOfSbs)
                (*recMb)[planeID][yInSbs * 8 + yOfSbs][xInSbs * 8 + xOfSbs] =
                    recMb8x8[planeID] [yInSbs * 8 + yOfSbs][xInSbs * 8 + xOfSbs] =
                    (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
    }
}

void IntraRec::reconstructure16x16(ColourComponent planeID)
{
    Intra16x16Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> recResBlk[4][4];

    uint8_t bitDepth = (planeID == COLOUR_COMPONENT_Y) ? bitDepthY : bitDepthC;

    setRefPixels16x16(planeID);

    pred16x16[planeID]->predict(predMode16x16[planeID], predBlk);

    tq.inverse16x16(planeID, mbEnc[planeID].coefs.blk16x16, recResBlk);

    SpliceBlock<16, 16>(predBlk, recResBlk, (*recMb)[planeID], bitDepth);
}

void IntraRec::reconstructureChroma()
{
    IntraChromaPredictor::PredBlk predBlk;

    for(int planeID = COLOUR_COMPONENT_CB; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
        setRefPixelsChroma((ColourComponent)planeID);

        if(chromaFormat == CHROMA_FORMAT_420) {
            Blk<ResType, 4, 4> resBlk[2][2];

            predChroma[planeID - 1]->predict((IntraChromaPredMode)mbEnc[COLOUR_COMPONENT_Y].intraChromaPredMode, predBlk);
            //printf("chromaPredMode : %d \n", mbEnc[COLOUR_COMPONENT_Y].intraChromaPredMode);
            tq.inverseChroma((ColourComponent)planeID, mbEnc[planeID].coefs.blk16x16, resBlk);

            SpliceBlock<8, 8>(predBlk, resBlk, (*recMb)[planeID], bitDepthC);
        } else {
            Blk<ResType, 4, 4> resBlk[4][2];

            predChroma[planeID - 1]->predict((IntraChromaPredMode)mbEnc[COLOUR_COMPONENT_Y].intraChromaPredMode, predBlk);

            tq.inverseChroma((ColourComponent)planeID, mbEnc[planeID].coefs.blk16x16, resBlk);

            SpliceBlock<8, 16>(predBlk, resBlk, (*recMb)[planeID], bitDepthC);
        }
    }
}