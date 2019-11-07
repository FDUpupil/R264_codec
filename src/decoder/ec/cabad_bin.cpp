#include"decoder/ec/cabad.h"
#include"common/block.h"

#define ORG_BS R"(org_bs.txt)"


uint8_t CABAD::biariDecodeRegular(ColourComponent compID, uint16_t ctxIdx)
{
	FILE* forg_bs;
	forg_bs = fopen(ORG_BS, "a");

    uint8_t     valMPS;
    uint16_t    range;
    uint16_t    rangeLPS;
    uint8_t     binsVal;

    range = engine[compID].getRange();
    valMPS = ctxModel[compID].getValMPS(ctxIdx);
    rangeLPS = ctxModel[compID].getRangeLPS(ctxIdx, range);

    binsVal = engine[compID].decodeRegular(valMPS, rangeLPS);
	printf("bit: %d | MPS: %d | LPS: %d | ctx: %d | range: %x \n", binsVal, valMPS, rangeLPS, ctxIdx, range);
    ctxModel[compID].update(ctxIdx, binsVal);

	fprintf(forg_bs, "(%d)", binsVal);
	fclose(forg_bs);

    return binsVal;
}

uint32_t CABAD::biariDecodeBypass(ColourComponent compID, uint8_t length)
{
	FILE* forg_bs;
	forg_bs = fopen(ORG_BS, "a");

	uint32_t     binsVal;

	binsVal = engine[compID].decodeBypass(length);

	fprintf(forg_bs, "[%d]", binsVal);
	fclose(forg_bs);

	return binsVal;
}

uint8_t CABAD::biariDecodeTerminate(ColourComponent compID)
{
	FILE* forg_bs;
	forg_bs = fopen(ORG_BS, "a");

    uint8_t     binsVal;

    binsVal = engine[compID].decodeTerminate();

	fprintf(forg_bs, "{%d}", binsVal);
	fclose(forg_bs);

    return binsVal;

}

uint32_t CABAD::biariDecodeFixedlLength(ColourComponent compID, uint8_t length, uint16_t ctxIdx)
{
    uint32_t binsVal = 0;
    while(length--) {
        binsVal << 1;
        binsVal |= (uint32_t)biariDecodeRegular(compID, ctxIdx);
    }
    return binsVal;
}

uint8_t CABAD::biariDecodeTruncatedUnary(ColourComponent compID, uint8_t bitMax, uint16_t ctxIdx, uint16_t ctxIdxMax)
{
    uint8_t binsVal = 0;
    uint8_t bit;
    uint8_t bitcount = 0;
    
    bit = biariDecodeRegular(compID, ctxIdx);
    bitcount++;
	binsVal += bit;

    while(bit != 0 && bitcount < bitMax) {
        if(ctxIdx < ctxIdxMax)
            ctxIdx++;
        bit = biariDecodeRegular(compID, ctxIdx);
        bitcount++;
		binsVal += bit;
    }
	//printf("binsVal: %d\n", binsVal);
    return binsVal;
}

uint8_t CABAD::biariDecodeUnary(ColourComponent compID, uint16_t ctxIdx, uint16_t ctxIdxMax)
{
    uint8_t binsVal = 0;
    while(biariDecodeRegular(compID, ctxIdxMax)) {
        binsVal++;
        if(ctxIdx < ctxIdxMax)
            ctxIdx++;
    }
    return binsVal;
}

uint32_t CABAD::biariDecodeUEG0(ColourComponent compID)
{
    uint32_t binsVal;
    uint32_t binVal;
    uint8_t length = 0;

    binVal = biariDecodeBypass(compID, 1U);

    while(binVal) {
        length++;
        binVal = biariDecodeBypass(compID, 1U);
    }

    if(length > 0) {
        binsVal = (1U << length) -1 + biariDecodeBypass(compID, length);
    }
    else 
        binsVal = 0;

    return binsVal;
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagLumaDC(ColourComponent planeID)
{
    return refFlags[planeID].nonZeroDC;
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagLumaAC(ColourComponent planeID, uint8_t idx)
{
	return getFlagForCodedBlockFlagLuma4x4(planeID, idx);
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagLuma4x4(ColourComponent planeID, uint8_t idx)
{
	uint8_t xInSbs = (uint8_t)MacroblockInvScan4x4[idx].x;
	uint8_t yInSbs = (uint8_t)MacroblockInvScan4x4[idx].y;
	uint8_t flagA;
	uint8_t flagB;

	if (xInSbs == 0)
		flagA = refFlags[planeID].nonZero.a[yInSbs];
	else
		flagA = !!(mb[planeID].nonZeroFlags & 1 << MacroblockScan4x4[yInSbs][xInSbs - 1]);

	if (yInSbs == 0)
		flagB = refFlags[planeID].nonZero.b[xInSbs];
	else
		flagB = !!(mb[planeID].nonZeroFlags & 1 << MacroblockScan4x4[yInSbs - 1][xInSbs]);

	return CordTermFlag2D<1>(flagA, flagB);
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagLuma8x8(ColourComponent planeID, uint8_t idx)
{
	ColourComponent compID = (chromaFormat == CHROMA_FORMAT_444 && !separateColourPlaneFlag) ?
		COLOUR_COMPONENT_Y : planeID;
	uint8_t xInSbs = (uint8_t)MacroblockInvScan2x2[idx].x;
	uint8_t yInSbs = (uint8_t)MacroblockInvScan2x2[idx].y;
	uint8_t flagA;
	uint8_t flagB;

	if (xInSbs == 0) {
		if (xInMbs == 0)
			flagA = 1;
		else if (refFlags[compID].transform8x8.a)
			flagA = refFlags[planeID].nonZero.a[yInSbs * 2 + 0] ||
			refFlags[planeID].nonZero.a[yInSbs * 2 + 1];
		else
			flagA = 0;
	}
	else {
		flagA = !!(mb[planeID].nonZeroFlags & 0xF << ((idx - 1) * 4));
	}

	if (yInSbs == 0) {
		if (yInMbs == 0)
			flagB = 1;
		else if (refFlags[compID].transform8x8.b)
			flagB = refFlags[planeID].nonZero.b[xInSbs * 2 + 0] ||
			refFlags[planeID].nonZero.b[xInSbs * 2 + 1];
		else
			flagB = 0;
	}
	else {
		flagB = !!(mb[planeID].nonZeroFlags & 0xF << ((idx - 2) * 4));
	}

	return CordTermFlag2D<1>(flagA, flagB);
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagChromaDC(ColourComponent planeID)
{
	return getFlagForCodedBlockFlagLumaDC(planeID);
}

CordTermFlag2D<1> CABAD::getFlagForCodedBlockFlagChromaAC(ColourComponent planeID, uint8_t idx)
{
	uint8_t xInSbs = (uint8_t)MacroblockInvScan2x4[idx].x;
	uint8_t yInSbs = (uint8_t)MacroblockInvScan2x4[idx].y;
	uint8_t flagA;
	uint8_t flagB;

	if (xInSbs == 0)
		flagA = refFlags[planeID].nonZero.a[yInSbs];
	else
		flagA = !!(mb[planeID].nonZeroFlags & 1 << MacroblockScan2x4[yInSbs][xInSbs - 1]);

	if (yInSbs == 0)
		flagB = refFlags[planeID].nonZero.b[xInSbs];
	else
		flagB = !!(mb[planeID].nonZeroFlags & 1 << MacroblockScan2x4[yInSbs - 1][xInSbs]);

	return CordTermFlag2D<1>(flagA, flagB);
}
