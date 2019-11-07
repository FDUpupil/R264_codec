#ifndef _ARITH_DEC_ENGINE_H_
#define _ARITH_DEC_ENGINE_H_

#include"tools/bitstream/bitstream.h"
#include<memory>

class ArithDecEngine {
public:
    void init();
    void setBitstream(Bitstream *_bs);

    uint8_t decodeRegular(uint8_t binMPS, uint8_t rangeLPS);
    uint32_t decodeBypass(uint8_t length);
    uint8_t decodeTerminate();

    uint32_t getValue() const {return value;}
    uint16_t getRange() const {return range;}

private:
    uint32_t    value;
    uint16_t    range;
    int8_t     bitleft;

    Bitstream *bs;

    uint32_t getWordFromBitsteam();
	uint32_t getByteFromBitsteam();
    uint8_t getOverflowBits(uint8_t rangeLPS);
};

#endif