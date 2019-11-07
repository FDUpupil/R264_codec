#include <utility>
#include <memory>

#include "decoder/top/decoder_top.h"
using namespace std;

#define BS_FILE R"(E:\WORK\R264_D\build\vs2019\R264_D\test.264)"


int main()
{
    int nalu_num = 0;
	int frame_num = 0;

    FILE *fbs;
    fbs = fopen(BS_FILE,"rb");

	DecoderTop decoderTop;

	decoderTop.decode(fbs);  
    
    fclose(fbs);

    return 0;
}