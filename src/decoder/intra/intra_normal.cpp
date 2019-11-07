#include "intra_normal.h"
#include "tools/image/distortion.h"

#include <cstring>

// Search order

struct IntraSearchOrder {
    int count;
    int mode[9];
};

static const IntraSearchOrder Intra4x4SearchOrder[16] = {
    { // neighbour == {}
        1,
        { INTRA_4x4_DC }
    }, { // neighbour == { LEFT }
        3,
        { INTRA_4x4_HORIZONTAL, INTRA_4x4_DC, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT }
        1,
        { INTRA_4x4_DC }
    }, { // neighbour == { LEFT, UP_LEFT }
        3,
        { INTRA_4x4_HORIZONTAL, INTRA_4x4_DC, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP }
        4,
        { INTRA_4x4_VERTICAL, INTRA_4x4_DC, INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP }
        6,
        { INTRA_4x4_VERTICAL, INTRA_4x4_HORIZONTAL, INTRA_4x4_DC,
        INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP }
        4,
        { INTRA_4x4_VERTICAL, INTRA_4x4_DC, INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP_LEFT, UP }
        9,
        { INTRA_4x4_VERTICAL, INTRA_4x4_HORIZONTAL, INTRA_4x4_DC,
        INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_DIAGONAL_DOWN_RIGHT, INTRA_4x4_VERTICAL_RIGHT,
        INTRA_4x4_HORIZONTAL_DOWN, INTRA_4x4_VERTICAL_LEFT, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP_RIGHT }
        1,
        { INTRA_4x4_DC }
    }, { // neighbour == { LEFT, UP_RIGHT }
        3,
        { INTRA_4x4_HORIZONTAL, INTRA_4x4_DC, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP_RIGHT }
        1,
        { INTRA_4x4_DC }
    }, { // neighbour == { LEFT, UP_LEFT, UP_RIGHT }
        3,
        { INTRA_4x4_HORIZONTAL, INTRA_4x4_DC, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP, UP_RIGHT }
        4,
        { INTRA_4x4_VERTICAL, INTRA_4x4_DC, INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP, UP_RIGHT }
        6,
        { INTRA_4x4_VERTICAL, INTRA_4x4_HORIZONTAL, INTRA_4x4_DC,
        INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT, INTRA_4x4_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP, UP_RIGHT }
        4,
        { INTRA_4x4_VERTICAL, INTRA_4x4_DC, INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP_LEFT, UP, UP_RIGHT }
        9,
        { INTRA_4x4_VERTICAL, INTRA_4x4_HORIZONTAL, INTRA_4x4_DC,
        INTRA_4x4_DIAGONAL_DOWN_LEFT, INTRA_4x4_DIAGONAL_DOWN_RIGHT, INTRA_4x4_VERTICAL_RIGHT,
        INTRA_4x4_HORIZONTAL_DOWN, INTRA_4x4_VERTICAL_LEFT, INTRA_4x4_HORIZONTAL_UP }
    }
};

static const IntraSearchOrder Intra8x8SearchOrder[16] = {
    { // neighbour == {}
        1,
        { INTRA_8x8_DC }
    }, { // neighbour == { LEFT }
        3,
        { INTRA_8x8_HORIZONTAL, INTRA_8x8_DC, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT }
        1,
        { INTRA_8x8_DC }
    }, { // neighbour == { LEFT, UP_LEFT }
        3,
        { INTRA_8x8_HORIZONTAL, INTRA_8x8_DC, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP }
        4,
        { INTRA_8x8_VERTICAL, INTRA_8x8_DC, INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP }
        6,
        { INTRA_8x8_VERTICAL, INTRA_8x8_HORIZONTAL, INTRA_8x8_DC,
        INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP }
        4,
        { INTRA_8x8_VERTICAL, INTRA_8x8_DC, INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP_LEFT, UP }
        9,
        { INTRA_8x8_VERTICAL, INTRA_8x8_HORIZONTAL, INTRA_8x8_DC,
        INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_DIAGONAL_DOWN_RIGHT, INTRA_8x8_VERTICAL_RIGHT,
        INTRA_8x8_HORIZONTAL_DOWN, INTRA_8x8_VERTICAL_LEFT, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP_RIGHT }
        1,
        { INTRA_8x8_DC }
    }, { // neighbour == { LEFT, UP_RIGHT }
        3,
        { INTRA_8x8_HORIZONTAL, INTRA_8x8_DC, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP_RIGHT }
        1,
        { INTRA_8x8_DC }
    }, { // neighbour == { LEFT, UP_LEFT, UP_RIGHT }
        3,
        { INTRA_8x8_HORIZONTAL, INTRA_8x8_DC, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP, UP_RIGHT }
        4,
        { INTRA_8x8_VERTICAL, INTRA_8x8_DC, INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP, UP_RIGHT }
        6,
        { INTRA_8x8_VERTICAL, INTRA_8x8_HORIZONTAL, INTRA_8x8_DC,
        INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT, INTRA_8x8_HORIZONTAL_UP }
    }, { // neighbour == { UP_LEFT, UP, UP_RIGHT }
        4,
        { INTRA_8x8_VERTICAL, INTRA_8x8_DC, INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_VERTICAL_LEFT }
    }, { // neighbour == { LEFT, UP_LEFT, UP, UP_RIGHT }
        9,
        { INTRA_8x8_VERTICAL, INTRA_8x8_HORIZONTAL, INTRA_8x8_DC,
        INTRA_8x8_DIAGONAL_DOWN_LEFT, INTRA_8x8_DIAGONAL_DOWN_RIGHT, INTRA_8x8_VERTICAL_RIGHT,
        INTRA_8x8_HORIZONTAL_DOWN, INTRA_8x8_VERTICAL_LEFT, INTRA_8x8_HORIZONTAL_UP }
    }
};

static const IntraSearchOrder Intra16x16SearchOrder[8] = {
    { // neighbour == {}
        1,
        { INTRA_16x16_DC }
    }, { // neighbour == { LEFT }
        2,
        { INTRA_16x16_HORIZONTAL, INTRA_16x16_DC }
    }, { // neighbour == { UP_LEFT }
        1,
        { INTRA_16x16_DC }
    }, { // neighbour == { LEFT, UP_LEFT }
        2,
        { INTRA_16x16_HORIZONTAL, INTRA_16x16_DC }
    }, { // neighbour == { UP }
        2,
        { INTRA_16x16_VERTICAL, INTRA_16x16_DC }
    }, { // neighbour == { LEFT, UP }
        3,
        { INTRA_16x16_VERTICAL, INTRA_16x16_HORIZONTAL, INTRA_16x16_DC }
    }, { // neighbour == { UP_LEFT, UP }
        2,
        { INTRA_16x16_HORIZONTAL, INTRA_16x16_DC }
    }, { // neighbour == { LEFT, UP_LEFT, UP }
        4,
        { INTRA_16x16_VERTICAL, INTRA_16x16_HORIZONTAL, INTRA_16x16_DC, INTRA_16x16_PLANE }
    }
};

static const IntraSearchOrder IntraChromaSearchOrder[8] = {
    { // neighbour == {}
        1,
        { INTRA_CHROMA_DC }
    }, { // neighbour == { LEFT }
        2,
        { INTRA_CHROMA_DC, INTRA_CHROMA_HORIZONTAL }
    }, { // neighbour == { UP_LEFT }
        1,
        { INTRA_CHROMA_DC }
    }, { // neighbour == { LEFT, UP_LEFT }
        2,
        { INTRA_CHROMA_DC, INTRA_CHROMA_HORIZONTAL }
    }, { // neighbour == { UP }
        2,
        { INTRA_CHROMA_DC, INTRA_CHROMA_VERTICAL }
    }, { // neighbour == { LEFT, UP }
        3,
        { INTRA_CHROMA_DC, INTRA_CHROMA_VERTICAL, INTRA_CHROMA_HORIZONTAL }
    }, { // neighbour == { UP_LEFT, UP }
        2,
        { INTRA_CHROMA_DC, INTRA_CHROMA_HORIZONTAL }
    }, { // neighbour == { LEFT, UP_LEFT, UP }
        4,
        { INTRA_CHROMA_DC, INTRA_CHROMA_VERTICAL, INTRA_CHROMA_HORIZONTAL, INTRA_CHROMA_PLANE }
    }
};

IntraNormal::IntraNormal(const SequenceLevelConfig &seqCfg, const PictureLevelConfig &picCfg)
    : IntraBase(seqCfg, picCfg)
{
}

void IntraNormal::estimate()
{
    for (int compID = 0; compID < compCount; ++compID) {
        cost4x4[compID] = MAX_COST;
        cost8x8[compID] = MAX_COST;
        cost16x16[compID] = MAX_COST;
    }

    estimate4x4(COLOUR_COMPONENT_Y);
    if (transform8x8ModeFlag)
        estimate8x8(COLOUR_COMPONENT_Y);
    estimate16x16(COLOUR_COMPONENT_Y);

    if (chromaFormat == CHROMA_FORMAT_444) {
        if (separateColourPlaneFlag) {
            for (int compID = COLOUR_COMPONENT_CB; compID <= COLOUR_COMPONENT_CR; ++compID) {
                estimate4x4((ColourComponent)compID);
                if (transform8x8ModeFlag)
                    estimate8x8((ColourComponent)compID);
                estimate16x16((ColourComponent)compID);
            }
        }
    } else if (chromaFormat != CHROMA_FORMAT_400)
        estimateChroma();
}

void IntraNormal::decidePartition()
{
    for (int compID = 0; compID < compCount; ++compID)
        if (cost16x16[compID] <= cost4x4[compID] && cost16x16[compID] <= cost8x8[compID])
            partition[compID] = INTRA_PARTITION_16x16;
        else if (transform8x8ModeFlag && cost8x8[compID] <= cost4x4[compID])
            partition[compID] = INTRA_PARTITION_8x8;
        else
            partition[compID] = INTRA_PARTITION_4x4;
}

// WARNING: estimate*() functions are modified but not checked
// There would be some bugs about compID & planeID

void IntraNormal::estimate4x4(ColourComponent compID)
{
    Intra4x4Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> rawResBlk;
    Blk<ResType, 4, 4> recResBlk;
    uint8_t bitDepth = (compID == 0) ? bitDepthY : bitDepthC;
    uint8_t qp = tq.getMacroblockQP(compID);

    cost4x4[compID] = 0;
    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;
        int predMode = getPredIntra4x4PredMode(compID, xInSbs, yInSbs);
        int nb = neighbour4x4[yInSbs][xInSbs];
        CostType minModeCost = MAX_COST;
        CostType curModeCost;

        if (chromaFormat == CHROMA_FORMAT_444 && !separateColourPlaneFlag) {
            // Estimate all the components together
            for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID)
                setRefPixels4x4((ColourComponent)planeID, recMb4x4[planeID], xInSbs, yInSbs);

            for (int predID = 0; predID < Intra4x4SearchOrder[nb].count; ++predID) {
                Intra4x4PredMode mode = (Intra4x4PredMode)Intra4x4SearchOrder[nb].mode[predID];
                curModeCost = 0;

                for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
                    pred4x4[planeID]->predict(mode, predBlk);
                    DiffBlock(orgMb4x4[planeID][yInSbs][xInSbs], predBlk, rawResBlk);
                    curModeCost += calcModeCost4x4(rawResBlk, predMode == mode, qp);
                }

                if (curModeCost < minModeCost) {
                    minModeCost = curModeCost;
                    predModes4x4[COLOUR_COMPONENT_Y][yInSbs][xInSbs] = mode;
                }
            }

            for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
                pred4x4[planeID]->predict(predModes4x4[COLOUR_COMPONENT_Y][yInSbs][xInSbs], predBlk);

                DiffBlock(orgMb4x4[planeID][yInSbs][xInSbs], predBlk, rawResBlk);

                tq.encode4x4((ColourComponent)planeID, rawResBlk, coefs4x4[planeID].blk4x4[idx], recResBlk);
                cost4x4[COLOUR_COMPONENT_Y] += calcRDCost4x4(rawResBlk, recResBlk, coefs4x4[planeID].blk4x4[idx], qp);

                for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
                    for (uint8_t xOfSbs = 0; xOfSbs < 4; ++xOfSbs)
                        recMb4x4[planeID][yInSbs * 4 + yOfSbs][xInSbs * 4 + xOfSbs] =
                            (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
            }
        } else {
            setRefPixels4x4(compID, recMb4x4[compID], xInSbs, yInSbs);

            for (int predID = 0; predID < Intra4x4SearchOrder[nb].count; ++predID) {
                Intra4x4PredMode mode = (Intra4x4PredMode)Intra4x4SearchOrder[nb].mode[predID];

                pred4x4[compID]->predict(mode, predBlk);
                DiffBlock(orgMb4x4[compID][yInSbs][xInSbs], predBlk, rawResBlk);
                curModeCost = calcModeCost4x4(rawResBlk, predMode == mode, qp);

                if (curModeCost < minModeCost) {
                    minModeCost = curModeCost;
                    predModes4x4[compID][yInSbs][xInSbs] = mode;
                }
            }

            pred4x4[compID]->predict(predModes4x4[compID][yInSbs][xInSbs], predBlk);

            DiffBlock(orgMb4x4[compID][yInSbs][xInSbs], predBlk, rawResBlk);

            tq.encode4x4(compID, rawResBlk, coefs4x4[compID].blk4x4[idx], recResBlk);
            cost4x4[compID] += calcRDCost4x4(rawResBlk, recResBlk, coefs4x4[compID].blk4x4[idx], qp);

            for (uint8_t yOfSbs = 0; yOfSbs < 4; ++yOfSbs)
                for (uint8_t xOfSbs = 0; xOfSbs < 4; ++xOfSbs)
                    recMb4x4[compID][yInSbs * 4 + yOfSbs][xInSbs * 4 + xOfSbs] =
                        (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
        }
    }
}

void IntraNormal::estimate8x8(ColourComponent compID)
{
    Intra8x8Predictor::PredBlk predBlk;
    Blk<ResType, 8, 8> rawResBlk;
    Blk<ResType, 8, 8> recResBlk;
    uint8_t bitDepth = (compID == 0) ? bitDepthY : bitDepthC;
    uint8_t qp = tq.getMacroblockQP(compID);

    cost8x8[compID] = 0;
    for (uint8_t idx = 0; idx < 4; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;
        int predMode = getPredIntra8x8PredMode(compID, xInSbs, yInSbs);
        int nb = neighbour8x8[yInSbs][xInSbs];
        CostType minModeCost = MAX_COST;
        CostType curModeCost;

        if (chromaFormat == CHROMA_FORMAT_444 && !separateColourPlaneFlag) {
            // Estimate all the components together
            for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID)
                setRefPixels8x8((ColourComponent)planeID, recMb8x8[planeID], xInSbs, yInSbs);

            for (int predID = 0; predID < Intra8x8SearchOrder[nb].count; ++predID) {
                Intra8x8PredMode mode = (Intra8x8PredMode)Intra8x8SearchOrder[nb].mode[predID];
                curModeCost = 0;

                for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
                    pred8x8[planeID]->predict(mode, predBlk);
                    DiffBlock(orgMb8x8[planeID][yInSbs][xInSbs], predBlk, rawResBlk);
                    curModeCost += calcModeCost8x8(rawResBlk, predMode == mode, qp);
                }

                if (curModeCost < minModeCost) {
                    minModeCost = curModeCost;
                    predModes8x8[COLOUR_COMPONENT_Y][yInSbs][xInSbs] = mode;
                }
            }

            for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
                pred8x8[planeID]->predict(predModes8x8[COLOUR_COMPONENT_Y][yInSbs][xInSbs], predBlk);

                DiffBlock(orgMb8x8[planeID][yInSbs][xInSbs], predBlk, rawResBlk);

                tq.encode8x8((ColourComponent)planeID, rawResBlk, coefs8x8[planeID].blk8x8[idx], recResBlk);
                cost8x8[COLOUR_COMPONENT_Y] += calcRDCost8x8(rawResBlk, recResBlk, coefs8x8[planeID].blk8x8[idx], qp);

                for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
                    for (uint8_t xOfSbs = 0; xOfSbs < 8; ++xOfSbs)
                        recMb8x8[planeID][yInSbs * 8 + yOfSbs][xInSbs * 8 + xOfSbs] =
                            (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
            }
        } else {
            setRefPixels8x8(compID, recMb8x8[compID], xInSbs, yInSbs);

            for (int predID = 0; predID < Intra8x8SearchOrder[nb].count; ++predID) {
                Intra8x8PredMode mode = (Intra8x8PredMode)Intra8x8SearchOrder[nb].mode[predID];

                pred8x8[compID]->predict(mode, predBlk);
                DiffBlock(orgMb8x8[compID][yInSbs][xInSbs], predBlk, rawResBlk);
                curModeCost = calcModeCost8x8(rawResBlk, predMode == mode, qp);

                if (curModeCost < minModeCost) {
                    minModeCost = curModeCost;
                    predModes8x8[compID][yInSbs][xInSbs] = mode;
                }
            }

            pred8x8[compID]->predict(predModes8x8[compID][yInSbs][xInSbs], predBlk);

            DiffBlock(orgMb8x8[compID][yInSbs][xInSbs], predBlk, rawResBlk);

            tq.encode8x8(compID, rawResBlk, coefs8x8[compID].blk8x8[idx], recResBlk);
            cost8x8[compID] += calcRDCost8x8(rawResBlk, recResBlk, coefs8x8[compID].blk8x8[idx], qp);

            for (uint8_t yOfSbs = 0; yOfSbs < 8; ++yOfSbs)
                for (uint8_t xOfSbs = 0; xOfSbs < 8; ++xOfSbs)
                    recMb8x8[compID][yInSbs * 8 + yOfSbs][xInSbs * 8 + xOfSbs] =
                        (PixType)Clip1(predBlk[yOfSbs][xOfSbs] + recResBlk[yOfSbs][xOfSbs], bitDepth);
        }
    }
}

void IntraNormal::estimate16x16(ColourComponent compID)
{
    Intra16x16Predictor::PredBlk predBlk;
    Blk<ResType, 4, 4> rawResBlk[4][4];
    Blk<ResType, 4, 4> recResBlk[4][4];
    uint8_t bitDepth = (compID == 0) ? bitDepthY : bitDepthC;
    uint8_t qp = tq.getMacroblockQP(compID);
    CostType minModeCost = MAX_COST;
    CostType curModeCost;

    cost16x16[compID] = 0;
    if (chromaFormat == CHROMA_FORMAT_444 && !separateColourPlaneFlag) {
        // Estimate all the components together
        for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID)
            setRefPixels16x16((ColourComponent)planeID);

        for (int predID = 0; predID < Intra16x16SearchOrder[neighbour16x16].count; ++predID) {
            Intra16x16PredMode mode = (Intra16x16PredMode)Intra16x16SearchOrder[neighbour16x16].mode[predID];
            curModeCost = 0;

            for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
                pred16x16[planeID]->predict(mode, predBlk);
                SplitBlock<16, 16>(orgMb16x16[planeID], predBlk, rawResBlk);
                curModeCost += calcModeCostFlatBlock<16, 16>(rawResBlk);
            }

            if (curModeCost < minModeCost) {
                minModeCost = curModeCost;
                predMode16x16[COLOUR_COMPONENT_Y] = mode;
            }
        }

        for (int planeID = 0; planeID < COLOUR_COMPONENT_COUNT; ++planeID) {
            pred16x16[planeID]->predict(predMode16x16[COLOUR_COMPONENT_Y], predBlk);

            SplitBlock<16, 16>(orgMb16x16[planeID], predBlk, rawResBlk);

            tq.encode16x16((ColourComponent)planeID, rawResBlk, coefs16x16[planeID].blk16x16, recResBlk);
            cost16x16[COLOUR_COMPONENT_Y] += calcRDCost16x16(rawResBlk, recResBlk, coefs16x16[planeID].blk16x16, qp);

            SpliceBlock<16, 16>(predBlk, recResBlk, recMb16x16[planeID], bitDepth);
        }
    } else {
        setRefPixels16x16(compID);

        for (int predID = 0; predID < Intra16x16SearchOrder[neighbour16x16].count; ++predID) {
            Intra16x16PredMode mode = (Intra16x16PredMode)Intra16x16SearchOrder[neighbour16x16].mode[predID];

            pred16x16[compID]->predict(mode, predBlk);
            SplitBlock<16, 16>(orgMb16x16[compID], predBlk, rawResBlk);
            curModeCost = calcModeCostFlatBlock<16, 16>(rawResBlk);

            if (curModeCost < minModeCost) {
                minModeCost = curModeCost;
                predMode16x16[compID] = mode;
            }
        }

        pred16x16[compID]->predict(predMode16x16[compID], predBlk);

        SplitBlock<16, 16>(orgMb16x16[compID], predBlk, rawResBlk);

        tq.encode16x16(compID, rawResBlk, coefs16x16[compID].blk16x16, recResBlk);
        cost16x16[compID] = calcRDCost16x16(rawResBlk, recResBlk, coefs16x16[compID].blk16x16, qp);

        SpliceBlock<16, 16>(predBlk, recResBlk, recMb16x16[compID], bitDepth);
    }
}

void IntraNormal::estimateChroma()
{
    IntraChromaPredictor::PredBlk predBlk;
    Blk<ResType, 4, 4> resBlk[4][2];
    CostType minModeCost = MAX_COST;
    CostType curModeCost;

    for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID)
        setRefPixelsChroma((ColourComponent)planeID);

    for (int predID = 0; predID < IntraChromaSearchOrder[neighbourChroma].count; ++predID) {
        IntraChromaPredMode mode = (IntraChromaPredMode)IntraChromaSearchOrder[neighbourChroma].mode[predID];
        curModeCost = 0;

        if (chromaFormat == CHROMA_FORMAT_420) {
            for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
                predChroma[planeID - 1]->predict(mode, predBlk);
                SplitBlock<8, 8>(orgMb16x16[planeID], predBlk, (Blk<ResType, 4, 4> (&)[2][2])resBlk);
                curModeCost += calcModeCostFlatBlock<8, 8>((Blk<ResType, 4, 4> (&)[2][2])resBlk);
            }
        } else {
            for (int planeID = COLOUR_COMPONENT_CB; planeID <= COLOUR_COMPONENT_CR; ++planeID) {
                predChroma[planeID - 1]->predict(mode, predBlk);
                SplitBlock<8, 16>(orgMb16x16[planeID], predBlk, resBlk);
                curModeCost += calcModeCostFlatBlock<8, 16>(resBlk);
            }
        }

        if (curModeCost < minModeCost) {
            minModeCost = curModeCost;
            predModeChroma = mode;
        }
    }
}

void IntraNormal::encodeCoefs4x4(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    std::memcpy(recComp, recMb4x4[planeID], sizeof(recComp));
    std::memcpy(&coefs, &coefs4x4[planeID], sizeof(coefs));
}

void IntraNormal::encodeCoefs8x8(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    std::memcpy(recComp, recMb8x8[planeID], sizeof(recComp));
    std::memcpy(&coefs, &coefs8x8[planeID], sizeof(coefs));
}

void IntraNormal::encodeCoefs16x16(ColourComponent planeID, Blk<PixType, 16, 16> &recComp, CompCoefs &coefs)
{
    std::memcpy(recComp, recMb16x16[planeID], sizeof(recComp));
    std::memcpy(&coefs, &coefs16x16[planeID], sizeof(coefs));
}

inline CostType IntraNormal::calcModeCost4x4(const Blk<ResType, 4, 4> &resBlk) const
{
    return SATD4x4(resBlk);
}

inline CostType IntraNormal::calcModeCost4x4(const Blk<ResType, 4, 4> &resBlk, bool sameMode, uint8_t qp) const
{
    return calcModeCost4x4(resBlk) + (sameMode ? 0 : qp);
}

inline CostType IntraNormal::calcModeCost8x8(const Blk<ResType, 8, 8> &resBlk) const
{
    return SATD8x8(resBlk);
}

inline CostType IntraNormal::calcModeCost8x8(const Blk<ResType, 8, 8> &resBlk, bool sameMode, uint8_t qp) const
{
    return calcModeCost8x8(resBlk) + (sameMode ? 0 : qp);
}

CostType IntraNormal::calcRDCost4x4(
    const Blk<ResType, 4, 4> &rawResBlk, const Blk<ResType, 4, 4> &recResBlk,
    const CoefType (&coefs)[16], uint8_t qp) const
{
    Unused(coefs);
    Unused(qp);
#if USE_PARTITION_SELECT_SATD
    return SATD4x4(rawResBlk, recResBlk);
#else
    return SAD4x4(rawResBlk, recResBlk);
#endif
}

CostType IntraNormal::calcRDCost8x8(
    const Blk<ResType, 8, 8> &rawResBlk, const Blk<ResType, 8, 8> &recResBlk,
    const CoefType (&coefs)[64], uint8_t qp) const
{
    Unused(coefs);
    Unused(qp);
#if USE_PARTITION_SELECT_SATD
    return SATD8x8(rawResBlk, recResBlk);
#else
    return SAD8x8(rawResBlk, recResBlk);
#endif
}

CostType IntraNormal::calcRDCost16x16(
    const Blk<ResType, 4, 4> (&rawResBlk)[4][4], const Blk<ResType, 4, 4> (&recResBlk)[4][4],
    const CompCoefs::Blk16x16Type &coefs, uint8_t qp) const
{
    CostType cost = 0;

    Unused(coefs);
    Unused(qp);

    for (uint8_t idx = 0; idx < 16; ++idx) {
        uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
        uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;
#if USE_PARTITION_SELECT_SATD 
		cost += SATD4x4(rawResBlk[yInSbs][xInSbs], recResBlk[yInSbs][xInSbs]);
#else
        cost += SAD4x4(rawResBlk[yInSbs][xInSbs], recResBlk[yInSbs][xInSbs]);
#endif
    }

    return cost;
}
