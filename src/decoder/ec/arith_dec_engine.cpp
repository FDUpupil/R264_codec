#include"arith_dec_engine.h"

void ArithDecEngine::init()
{
    value = getByteFromBitsteam();
    value = (value << 16) | getWordFromBitsteam();

    bitleft = 15;
    range = 0x1FE;
}

void ArithDecEngine::setBitstream(Bitstream *_bs)
{
    bs = _bs;
}

uint8_t ArithDecEngine::decodeRegular(uint8_t bitMPS, uint8_t rangeLPS)
{
    uint8_t bit = bitMPS;
    range -= rangeLPS;
	//printf("value: %x bitleft: %d\n", value, bitleft);
	//printf("range: %x\n", range);
	//printf("bitleft: %d\n", bitleft);
    if(value < (range << bitleft)){
        if(range >= 256)
            return bit;
        else {
            range <<= 1;
            bitleft--;
        }
    }
    else {
        uint8_t renorm = getOverflowBits(rangeLPS);
        value -= (range << bitleft);
		//printf("LPS\n");
        range = rangeLPS << renorm;
        bitleft -= renorm;

        bit ^= 0x01;
    }

    if(bitleft > 0)
        return bit;
    else {
        value <<= 16;
        value |= getWordFromBitsteam();

        bitleft += 16;	
        return bit;
    }
}

uint32_t ArithDecEngine::decodeBypass(uint8_t length)
{
	uint32_t binsVal = 0;
	while (length > 0) {
		bitleft--;
		
		length--;
        
		if (value < (range << bitleft)) {
			//printf("decode one bit 0, bitleft:%d\n", bitleft);
			binsVal = (binsVal << 1) | 0;
		}
		else {
			//printf("decode one bit 1, bitleft:%d\n", bitleft);
			binsVal = (binsVal << 1) | 1;
			value -= (range << bitleft);
		}
		if (bitleft == 0) {
			value <<= 16;
			value |= getWordFromBitsteam();

			bitleft = 16;
		}
	}
	
	return binsVal;
}

// uint32_t ArithDecEngine::decodeBypass(uint8_t length)
// {
//     uint32_t binsVal = 0;
//     while(length > 0) {
//         if(length <= bitleft) {
//             binsVal = (binsVal << length) | ((value << length) / range);
//             bitleft -= length;
// 			length = 0;
//         }
//         else {
//             binsVal = (binsVal << bitleft) | ((value << bitleft) / range);
//             length -= bitleft;
//             bitleft = 0;
//         }
//         if (bitleft <= 0) {
// 			value <<= 16;
// 			value |= get2ByteFromBitsteam();

// 			bitleft += 16;
// 		}
//     }
//     return binsVal;
// }

uint8_t ArithDecEngine::decodeTerminate()
{
    uint8_t bit = 0;
    uint16_t rangeLPS = 2;

    range -= rangeLPS;

    if(value < (range << bitleft)){
        if(range >= 256)
            return bit;
        else {
            range <<= 1;
            bitleft--;
        }
    }
    else {
        uint8_t renorm = getOverflowBits(rangeLPS);
        value -= (range << bitleft);

        range = rangeLPS << renorm;
        bitleft -= renorm;

        bit ^= 0x01;
    }

    if(bitleft > 0)
        return bit;
    else {
        value <<= 16;
        value |= getWordFromBitsteam();

        bitleft += 16;

        return bit;
    }

}

uint32_t ArithDecEngine::getWordFromBitsteam()
{
    return bs->readFixedLength(16);
}

uint32_t ArithDecEngine::getByteFromBitsteam()
{
	return bs->readFixedLength(8);

}
uint8_t ArithDecEngine::getOverflowBits(uint8_t rangeLPS)
{
    // range is between 6 and 255
    static const uint8_t LeadingZeros[256 >> 3] = {
        6, 5, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
    };

    return LeadingZeros[rangeLPS >> 3];
}