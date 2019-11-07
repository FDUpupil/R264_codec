#ifndef _ANNEXB_H_
#define _ANNEXB_H_

#include <vector>
#include <memory>
#include"bitstream.h"

class Annex_b {
public:
	Annex_b(FILE *bitstreamfile);

	uint32_t getNalu(Bitstream &nalu);
	bool isEndofile() { return iseof; }
private:
	FILE *bitstreamptr;
	std::vector<uint8_t> iobitstreambuf;
	std::vector<uint8_t>::iterator iobitstreambufiter;
	uint32_t maxbtyeinbuf;
	uint32_t btyeinbuf;
	uint8_t nextstartcode;
	bool iseof;

	uint8_t getfbtye();
	uint32_t getChunk();
	//bool findStartcode(std::vector<uint8_t>::iterator &bufiter, uint8_t zerobit);

};

inline bool findStartcode(std::vector<uint8_t> &buf, uint8_t zerobit)
{
	if (buf.size() < (uint8_t)(zerobit + 1))
		return false;
	std::vector<uint8_t>::iterator bufiter = buf.end() - (zerobit + 1);
	for (int i = 0; i < zerobit; i++) {
		if (*(bufiter + i) != 0)
			return false;
	}

	if (*(bufiter + zerobit) != 1)
		return false;

	return true;
}

inline void removeExtraByte(std::vector<uint8_t> &buf)
{
	if (buf.size() < (uint8_t)4)
		return;
	std::vector<uint8_t>::iterator bufiter = buf.end() - 4;
	if(*bufiter == 0x00 && *(bufiter+1) == 0x00 && *(bufiter+2) == 0x03 && 
		(*(bufiter+3) == 0x00 || *(bufiter+3) == 0x01 || *(bufiter+3) == 0x02 || *(bufiter+3) == 0x03)) {
			uint8_t temp = *(bufiter+3);
			buf.erase(bufiter+2, buf.end());
			buf.emplace_back(temp);
		}
}

#endif