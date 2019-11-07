#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <vector>
#include <cassert>

#define BitstreamCheck(expr) assert(expr)

enum NALUType {
	NALU_TYPE_SLICE = 1,
	NALU_TYPE_DPA = 2,
	NALU_TYPE_DPB = 3,
	NALU_TYPE_DPC = 4,
	NALU_TYPE_IDR = 5,
	NALU_TYPE_SEI = 6,
	NALU_TYPE_SPS = 7,
	NALU_TYPE_PPS = 8,
	NALU_TYPE_AUD = 9,
	NALU_TYPE_EOSEQ = 10,
	NALU_TYPE_EOSTREAM = 11,
	NALU_TYPE_FILL = 12
};

class Bitstream {
public:
    Bitstream();

    void clear();
	void init();
	void align();
	bool bufempty() {return buf.empty();}
	const std::vector<uint8_t>& getBitstream() const { return buf; }
	std::vector<uint8_t>& getBitstream() { return buf; }

	NALUType getNalutype();
	bool readOneBit();
	uint32_t readFixedLength(uint8_t length);
	uint32_t readNextUEG0();
	int32_t  readNextSEG0();


private:
    std::vector<uint8_t> buf;
	std::vector<uint8_t>::iterator bufiter;
    uint8_t lengthLeft;

};

#endif