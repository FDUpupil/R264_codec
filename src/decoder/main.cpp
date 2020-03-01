#include <utility>
#include <memory>

#include "decoder/top/decoder_top.h"
using namespace std;

#define BS_FILE R"(F:\WORK_vip\R264_test\test.264)"
#define REC_FILE R"(F:\WORK_vip\R264_test\rec.yuv)"


int main()
{
    int nalu_num = 0;
	int frame_num = 0;

    FILE *fbs;
    FILE *frec;

    fbs = fopen(BS_FILE,"rb");
    frec = fopen(REC_FILE, "wb"); 

	DecoderTop decoderTop;

	decoderTop.decode(fbs, frec);  
    
    fclose(fbs);
	fclose(frec);

    return 0;
}