#include"bitstream.h"

#define NAL_REF_IDC_SHIFT 5
#define NAL_REF_IDC_BIT 0x3
#define NAL_UNIT_TYPE_BIT 0x1f

Bitstream::Bitstream()
	: lengthLeft(0)
{
}

void Bitstream::clear()
{
    buf.clear();
    lengthLeft = 0;
}

void Bitstream::init()
{
	if (!bufempty())
		bufiter = buf.begin();
}

void Bitstream::align_weak()
{
	if (!bufempty() && (lengthLeft < 8)) {
		lengthLeft = 8;
		if(bufiter != (buf.end() - 1)){
			bufiter++;
		}
	}
	//printf("byte : %x \n", *bufiter);
}

void Bitstream::align()
{
	if (!bufempty()) {
		lengthLeft = 8;
		if (bufiter != (buf.end() - 1)) {
			bufiter++;
		}
	}
	//printf("byte : %x \n", *bufiter);
}

NALUType Bitstream::getNalutype()
{
	uint8_t nalu_header = *bufiter;
	uint8_t nal_ref_idc = (nalu_header >> NAL_REF_IDC_SHIFT) & NAL_REF_IDC_BIT;
	uint8_t nal_unit_type = nalu_header & NAL_UNIT_TYPE_BIT;
	bufiter++;
	lengthLeft = 8;
	return (NALUType)nal_unit_type;
}

bool Bitstream::readOneBit()
{
	return (bool)(readFixedLength(1));
}

uint32_t Bitstream::readFixedLength(uint8_t length)
{
	uint32_t binsval = 0;
	if(length == 0)
		return binsval;
	if(length <= lengthLeft) {
		lengthLeft -= length;
		binsval = (*bufiter >> lengthLeft) & (uint32_t)((1 << length) - 1);
	}
	else {
		binsval = *bufiter & (uint32_t)((1 << lengthLeft) - 1);
		length -= lengthLeft;
		align();
		while(length > 8){
			binsval = (binsval << 8) | (uint32_t) (*bufiter);
			align();
			length -= 8;
		}
		lengthLeft  = 8 - length;
		binsval =  (binsval << length) | (uint32_t)(*bufiter >> lengthLeft);
	}
	if(lengthLeft == 0){
		align();
	}
	return binsval;
}

uint32_t Bitstream::readNextUEG0()
{
	uint8_t length = 0;
	uint32_t binsVal;
	while(!readOneBit()){
		length++;
	}
	binsVal = ((uint32_t)(1U << length) | readFixedLength(length)) - 1;

	return binsVal;
}

int32_t  Bitstream::readNextSEG0()
{
	uint32_t binsval = readNextUEG0();
	return (((binsval % 2) == 0)? (-(int32_t)(binsval / 2)) : (int32_t)((binsval + 1) / 2));

}