#include "mem_ctrl.h"

#include <memory>

MemCtrl::MemCtrl(const PictureLevelConfig &cfgPic)
    : chromaFormat(cfgPic.chromaFormat),
      widthInMbs(cfgPic.widthInMbs),
      heightInMbs(cfgPic.heightInMbs),
      separateColourPlaneFlag(cfgPic.separateColourPlaneFlag),
      entropyCodingModeFlag(cfgPic.entropyCodingModeFlag)
{
    compCount = (uint8_t)(separateColourPlaneFlag ? COLOUR_COMPONENT_COUNT : 1);
    planeCount = (uint8_t)((chromaFormat != CHROMA_FORMAT_400) ? COLOUR_COMPONENT_COUNT : 1);

    lastHorLinePixels[COLOUR_COMPONENT_Y] = new PixType[widthInMbs * 16];
    lastVerLinePixels[COLOUR_COMPONENT_Y] = new PixType[16];
    lastUpLeftPixels[COLOUR_COMPONENT_Y]  = new PixType[1];
    lastHorLineIntraPredModes[COLOUR_COMPONENT_Y] = new uint8_t[widthInMbs * 4];
    lastVerLineIntraPredModes[COLOUR_COMPONENT_Y] = new uint8_t[4];

    //mbWC = (chromaFormat != CHROMA_FORMAT_444) ? 8 : 16;
    //mbHC = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

    if (chromaFormat != CHROMA_FORMAT_400) {
        uint8_t mbWC = (chromaFormat != CHROMA_FORMAT_444) ? 8 : 16;
        uint8_t mbHC = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

        for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
            lastHorLinePixels[planeID] = new PixType[widthInMbs * mbWC];
            lastVerLinePixels[planeID] = new PixType[mbHC];
            lastUpLeftPixels[planeID]  = new PixType[1];
            lastHorLineIntraPredModes[planeID] = separateColourPlaneFlag ?
                new uint8_t[widthInMbs * 4] : nullptr;
            lastVerLineIntraPredModes[planeID] = separateColourPlaneFlag ?
                new uint8_t[4] : nullptr;
        }
    }

    if (!entropyCodingModeFlag) {
        // CAVLC
        for (int planeID = 0; planeID < planeCount; ++planeID) {
            leftNonZeroCount[planeID] = new uint8_t[4];
            upNonZeroCount[planeID] = new uint8_t[widthInMbs * 4];
        }
    } else {
        // CABAC
        for (int compID = 0; compID < compCount; ++compID) {
            prevMbQPDelta[compID] = new uint8_t[1];

            leftNonIntraNxN[compID] = new uint8_t[1];
            upNonIntraNxN[compID] = new uint8_t[widthInMbs];

            leftIntraChromaPredMode[compID] = new uint8_t[1];
            upIntraChromaPredMode[compID] = new uint8_t[widthInMbs];

            leftTransform8x8[compID] = new uint8_t[1];
            upTransform8x8[compID] = new uint8_t[widthInMbs];

            leftCBPLuma[compID] = new uint8_t[2];
            upCBPLuma[compID] = new uint8_t[widthInMbs * 2];

            leftCBPChroma[compID] = new uint8_t[1];
            upCBPChroma[compID] = new uint8_t[widthInMbs];
        }

        for (int planeID = 0; planeID < planeCount; ++planeID) {
            leftNonZero[planeID] = new uint8_t[4];
            upNonZero[planeID] = new uint8_t[widthInMbs * 4];

            leftNonZeroDC[planeID] = new uint8_t[1];
            upNonZeroDC[planeID] = new uint8_t[widthInMbs];
        }
    }

    // Deblocking Filter
    for (int compID = 0; compID < compCount; ++compID) {
        leftQP[compID] = new int8_t[1];
        upQP[compID] = new int8_t[widthInMbs];
    }
}

MemCtrl::~MemCtrl()
{
    for (int planeID = 0; planeID < planeCount; ++planeID) {
        delete[] lastHorLinePixels[planeID];
        delete[] lastVerLinePixels[planeID];
        delete[] lastUpLeftPixels[planeID];
        delete[] lastHorLineIntraPredModes[planeID];
        delete[] lastVerLineIntraPredModes[planeID];
    }

    if (!entropyCodingModeFlag) {
        // CAVLC
        for (int planeID = 0; planeID < planeCount; ++planeID) {
            delete[] leftNonZeroCount[planeID];
            delete[] upNonZeroCount[planeID];
        }
    } else {
        // CABAC
        for (int compID = 0; compID < compCount; ++compID) {
            delete[] prevMbQPDelta[compID];

            delete[] leftNonIntraNxN[compID];
            delete[] upNonIntraNxN[compID];

            delete[] leftIntraChromaPredMode[compID];
            delete[] upIntraChromaPredMode[compID];

            delete[] leftTransform8x8[compID];
            delete[] upTransform8x8[compID];

            delete[] leftCBPLuma[compID];
            delete[] upCBPLuma[compID];

            delete[] leftCBPChroma[compID];
            delete[] upCBPChroma[compID];
        }

        for (int planeID = 0; planeID < planeCount; ++planeID) {
            delete[] leftNonZero[planeID];
            delete[] upNonZero[planeID];

            //delete[] leftNonIntraNxN[planeID];
            //delete[] upNonIntraNxN[planeID];
        }
    }

    // Deblocking Filter
    for (int compID = 0; compID < compCount; ++compID) {
        delete[] leftQP[compID];
        delete[] upQP[compID];
    }
}

void MemCtrl::init(const SliceLevelConfig &cfgSilc)
{
    Unused(cfgSilc);

    xInMbs = 0;
    yInMbs = 0;
}

void MemCtrl::cycle()
{
    // Simply do nothing
}

void MemCtrl::requireOrgFrame(std::unique_ptr<BlockyImage> _orgFrame)
{
    MemCtrlCheck(
        (*_orgFrame).getWidth() == widthInMbs &&
        (*_orgFrame).getHeight() == heightInMbs
    );

    orgFrame = std::move(_orgFrame);
    curOrgMb = &(*orgFrame)[0][0];
}

void MemCtrl::requireRecFrame(std::unique_ptr<BlockyImage> _recFrame)
{
    MemCtrlCheck(
        (*_recFrame).getWidth() == widthInMbs &&
        (*_recFrame).getHeight() == heightInMbs
    );

    recFrame = std::move(_recFrame);
    curRecMb = &(*recFrame)[0][0];
}

std::unique_ptr<BlockyImage> MemCtrl::releaseOrgFrame()
{
    return std::move(orgFrame);
}

std::unique_ptr<BlockyImage> MemCtrl::releaseRecFrame()
{
    return std::move(recFrame);
}

void MemCtrl::getIntraMemory(MemCtrlToIntra &memIn, IntraToMemCtrl &memOut)
{
    getIntraPixels();
    getIntraPredModes();

    memIn.orgMb = curOrgMb;
    memIn.refPixels = curRefPixels;
    memIn.refModes = curRefIntraModes;

    memOut.recMb = curRecMb;
    memOut.predModes = curIntraPredModes;
}

void MemCtrl::getIntraPixels()
{
    uint8_t mbWC = (chromaFormat != CHROMA_FORMAT_444) ? 8 : 16;
    uint8_t mbHC = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;
    uint8_t mbW[COLOUR_COMPONENT_COUNT] = { 16, mbWC, mbWC };
    uint8_t mbH[COLOUR_COMPONENT_COUNT] = { 16, mbHC, mbHC };

    for (int planeID = 0; planeID < planeCount; ++planeID) {
        uint8_t refWidth;
        uint16_t xO = xInMbs * mbW[planeID];

        if (xInMbs == widthInMbs - 1)
            refWidth = mbW[planeID];
        else if (planeID == COLOUR_COMPONENT_Y || chromaFormat == CHROMA_FORMAT_444)
            refWidth = 16 + 8;
        else
            refWidth = mbW[planeID];

        for (uint16_t yOfMbs = 0; yOfMbs < mbH[planeID]; ++yOfMbs)
            curRefPixels[planeID].l[yOfMbs] = lastVerLinePixels[planeID][yOfMbs];
        if (xInMbs != 0)
            curRefPixels[planeID].ul = lastUpLeftPixels[planeID][0];
        for (uint16_t xOfMbs = 0; xOfMbs < refWidth; ++xOfMbs)
            curRefPixels[planeID].u[xOfMbs] = lastHorLinePixels[planeID][xO + xOfMbs];
    }
}

void MemCtrl::getIntraPredModes()
{
    uint16_t xO = xInMbs * 4;

    for (int compID = 0; compID < compCount; ++compID) {
        for (uint16_t yInSbs = 0; yInSbs < 4; ++yInSbs)
            curRefIntraModes[compID].l[yInSbs] = lastVerLineIntraPredModes[compID][yInSbs];
        for (uint16_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            curRefIntraModes[compID].u[xInSbs] = lastHorLineIntraPredModes[compID][xO + xInSbs];
    }
}

void MemCtrl::getECMemory(MemCtrlToEC &memIn, ECToMemCtrl &memOut)
{
    if (!entropyCodingModeFlag) {
        getCAVLCRefCounts();
        memIn.cavlcRefCounts = refCounts;
        memOut.cavlcCurCounts = curCounts;
    } else {
        getCABACRefFlags();
        memIn.cabacRefFlags = refFlags;
        memOut.cabacCurFlags = curFlags;
    }
}

void MemCtrl::getCAVLCRefCounts()
{
    for (int planeID = 0; planeID < planeCount; ++planeID) {
        for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
            refCounts[planeID].nA[yInSbs] = (xInMbs == 0) ? 0 : leftNonZeroCount[planeID][yInSbs];
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            refCounts[planeID].nB[xInSbs] = (yInMbs == 0) ? 0 : upNonZeroCount[planeID][xInMbs * 4 + xInSbs];
    }
}

void MemCtrl::getCABACRefFlags()
{
    for (int compID = 0; compID < compCount; ++compID) {
        // TODO: mb_qp_delta can be derived from qp cache
        refFlags[compID].mbQPDelta = (xInMbs == 0 && yInMbs == 0) ? 0 : prevMbQPDelta[compID][0];
        
        refFlags[compID].nonIntraNxN.a = (xInMbs == 0) ? 0 : leftNonIntraNxN[compID][0];
        refFlags[compID].nonIntraNxN.b = (yInMbs == 0) ? 0 : upNonIntraNxN[compID][xInMbs];

        refFlags[compID].intraChromaPredMode.a = (xInMbs == 0) ? 0 : leftIntraChromaPredMode[compID][0];
        refFlags[compID].intraChromaPredMode.b = (yInMbs == 0) ? 0 : upIntraChromaPredMode[compID][xInMbs];

        refFlags[compID].transform8x8.a = (xInMbs == 0) ? 0 : leftTransform8x8[compID][0];
        refFlags[compID].transform8x8.b = (yInMbs == 0) ? 0 : upTransform8x8[compID][xInMbs];

        refFlags[compID].cbpLuma.a[0] = (xInMbs == 0) ? 1 : leftCBPLuma[compID][0];
        refFlags[compID].cbpLuma.a[1] = (xInMbs == 0) ? 1 : leftCBPLuma[compID][1];
        refFlags[compID].cbpLuma.b[0] = (yInMbs == 0) ? 1 : upCBPLuma[compID][xInMbs * 2 + 0];
        refFlags[compID].cbpLuma.b[1] = (yInMbs == 0) ? 1 : upCBPLuma[compID][xInMbs * 2 + 1];

        refFlags[compID].cbpChroma.a = (xInMbs == 0) ? 0 : leftCBPChroma[compID][0];
        refFlags[compID].cbpChroma.b = (yInMbs == 0) ? 0 : upCBPChroma[compID][xInMbs];
    }

    for (int planeID = 0; planeID < planeCount; ++planeID) {
        // Assume that currMbAddr uses Intra Prediction if mbAddrN does not exist

        for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
            refFlags[planeID].nonZero.a[yInSbs] = (xInMbs == 0) ? 1 : leftNonZero[planeID][yInSbs];
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            refFlags[planeID].nonZero.b[xInSbs] = (yInMbs == 0) ? 1 : upNonZero[planeID][xInMbs * 4 + xInSbs];

        refFlags[planeID].nonZeroDC.a = (xInMbs == 0) ? 1 : leftNonZeroDC[planeID][0];
        refFlags[planeID].nonZeroDC.b = (yInMbs == 0) ? 1 : upNonZeroDC[planeID][xInMbs];
    }
}

void MemCtrl::getDbMemory(MemCtrlToDb &memIn, DbToMemCtrl &memOut)
{
    getDbRefQP();

    memIn.refQP = refQP;
    memIn.curMb = curRecMb;
    memIn.leftMb = (xInMbs != 0) ? curRecMb - 1 : nullptr;
    memIn.upMb = (yInMbs != 0) ? curRecMb - widthInMbs : nullptr;
    
    memOut.curQP = curQP;
    memOut.curMb = curRecMb;
    memOut.leftMb = (xInMbs != 0) ? curRecMb - 1 : nullptr;
    memOut.upMb = (yInMbs != 0) ? curRecMb - widthInMbs : nullptr;
}

void MemCtrl::getDbRefQP()
{
    for (int compID = 0; compID < compCount; ++compID) {
        refQP[compID].left = leftQP[compID][0];
        refQP[compID].up = upQP[compID][xInMbs];
    }
}

void MemCtrl::updateIntraMemory()
{
    updateIntraPixels();
    updateIntraPredModes();
}

void MemCtrl::updateECMemory()
{
    if (!entropyCodingModeFlag)
        updateCAVLCMemory();
    else
        updateCABACMemory();
}

void MemCtrl::updateDbMemory()
{
    // Reconstruct image is not necessary to update
    for (int compID = 0; compID < compCount; ++compID) {
        leftQP[compID][0] = curQP[compID];
        upQP[compID][xInMbs] = curQP[compID];
    }
}

void MemCtrl::setNextMb()
{
    if (++xInMbs == widthInMbs) {
        xInMbs = 0;
        ++yInMbs;
    }

    ++curOrgMb;
    ++curRecMb;
}

void MemCtrl::updateIntraPixels()
{
    uint8_t mbWC = (chromaFormat != CHROMA_FORMAT_444) ? 8 : 16;
    uint8_t mbHC = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;
    uint8_t mbW[COLOUR_COMPONENT_COUNT] = { 16, mbWC, mbWC };
    uint8_t mbH[COLOUR_COMPONENT_COUNT] = { 16, mbHC, mbHC };
    uint16_t xO;

    for (int planeID = 0; planeID < planeCount; ++planeID) {
        xO = xInMbs * mbW[planeID];
        lastUpLeftPixels[planeID][0] = lastHorLinePixels[planeID][xO + mbW[planeID] - 1];
        for (uint16_t xOfMbs = 0; xOfMbs < mbW[planeID]; ++xOfMbs)
            lastHorLinePixels[planeID][xO + xOfMbs] = (*curRecMb)[planeID][mbH[planeID] - 1][xOfMbs];
        for (uint16_t yOfMbs = 0; yOfMbs < mbH[planeID]; ++yOfMbs)
            lastVerLinePixels[planeID][yOfMbs] = (*curRecMb)[planeID][yOfMbs][mbW[planeID] - 1];
    }
}

void MemCtrl::updateIntraPredModes()
{
    uint16_t xO = xInMbs * 4;

    for (int compID = 0; compID < compCount; ++compID) {
        for (uint16_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            lastHorLineIntraPredModes[compID][xO + xInSbs] = (uint8_t)curIntraPredModes[compID][3][xInSbs];
        for (uint16_t yInSbs = 0; yInSbs < 4; ++yInSbs)
            lastVerLineIntraPredModes[compID][yInSbs] = (uint8_t)curIntraPredModes[compID][yInSbs][3];
    }
}

void MemCtrl::updateCAVLCMemory()
{
    uint8_t sbWC = (chromaFormat != CHROMA_FORMAT_444) ? 2 : 4;
    uint8_t sbHC = (chromaFormat == CHROMA_FORMAT_420) ? 2 : 4;
    uint8_t sbW[COLOUR_COMPONENT_COUNT] = { 4, sbWC, sbWC };
    uint8_t sbH[COLOUR_COMPONENT_COUNT] = { 4, sbHC, sbHC };

    for (int planeID = 0; planeID < planeCount; ++planeID) {
        for (uint8_t yInSbs = 0; yInSbs < 4; ++yInSbs)
            leftNonZeroCount[planeID][yInSbs] = curCounts[planeID][yInSbs][sbW[planeID] - 1];
        for (uint8_t xInSbs = 0; xInSbs < 4; ++xInSbs)
            upNonZeroCount[planeID][xInMbs * 4 + xInSbs] = curCounts[planeID][sbH[planeID] - 1][xInSbs];
    }
 }

void MemCtrl::updateCABACMemory()
{
    for (int compID = 0; compID < compCount; ++compID) {
        prevMbQPDelta[compID][0] = curFlags[compID].mbQPDelta;

        leftNonIntraNxN[compID][0] = curFlags[compID].nonIntraNxN;
        upNonIntraNxN[compID][xInMbs] = curFlags[compID].nonIntraNxN;

        leftIntraChromaPredMode[compID][0] = curFlags[compID].intraChromaPredMode;
        upIntraChromaPredMode[compID][xInMbs] = curFlags[compID].intraChromaPredMode;

        leftTransform8x8[compID][0] = curFlags[compID].transform8x8;
        upTransform8x8[compID][xInMbs] = curFlags[compID].transform8x8;

        leftCBPLuma[compID][0] = !!(curFlags[compID].cbpLuma & 1 << MacroblockScan2x2[0][1]);
        leftCBPLuma[compID][1] = !!(curFlags[compID].cbpLuma & 1 << MacroblockScan2x2[1][1]);
        upCBPLuma[compID][xInMbs * 2 + 0] = !!(curFlags[compID].cbpLuma & 1 << MacroblockScan2x2[1][0]);
        upCBPLuma[compID][xInMbs * 2 + 1] = !!(curFlags[compID].cbpLuma & 1 << MacroblockScan2x2[1][1]);

        leftCBPChroma[compID][0] = curFlags[compID].cbpChroma;
        upCBPChroma[compID][xInMbs] = curFlags[compID].cbpChroma;
    }

    for (int planeID = 0; planeID < planeCount; ++planeID) {
        uint16_t xO = xInMbs * 4;
        if (chromaFormat == CHROMA_FORMAT_444 || planeID == COLOUR_COMPONENT_Y) {
            for (int yInSbs = 0; yInSbs < 4; ++yInSbs)
                leftNonZero[planeID][yInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan4x4[yInSbs][3]);
            for (int xInSbs = 0; xInSbs < 4; ++xInSbs)
                upNonZero[planeID][xO + xInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan4x4[3][xInSbs]);
        } else if (chromaFormat == CHROMA_FORMAT_420) {
            for (int yInSbs = 0; yInSbs < 2; ++yInSbs)
                leftNonZero[planeID][yInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan2x2[yInSbs][1]);
            for (int xInSbs = 0; xInSbs < 2; ++xInSbs)
                upNonZero[planeID][xO + xInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan2x2[1][xInSbs]);
        } else { // chromaFormat == CHROMA_FORMAT_422
            for (int yInSbs = 0; yInSbs < 4; ++yInSbs)
                leftNonZero[planeID][yInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan2x4[yInSbs][1]);
            for (int xInSbs = 0; xInSbs < 2; ++xInSbs)
                upNonZero[planeID][xO + xInSbs] = !!(curFlags[planeID].nonZero & 1 << MacroblockScan2x4[3][xInSbs]);
        }

        leftNonZeroDC[planeID][0] = (uint8_t)(curFlags[planeID].nonZero >> NON_ZERO_FLAG_DC_SHIFT);
        upNonZeroDC[planeID][xInMbs] = (uint8_t)(curFlags[planeID].nonZero >> NON_ZERO_FLAG_DC_SHIFT);
    }
}

void MemCtrl::getRecMemory(MemCtrlToRec &memIn, RecToMemCtrl &memOut) 
{
    getIntraPixels();
    getIntraPredModes();

    memIn.refPixels = curRefPixels; 
    memIn.refModes = curRefIntraModes;

    memOut.recMb = curRecMb;
    memOut.predModes = curIntraPredModes;
}

