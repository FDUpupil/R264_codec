#include"annex_b.h"

#define IOBUFFERSIZE 65536//512*1024

Annex_b::Annex_b (FILE *bitstreamfile)
    : bitstreamptr (bitstreamfile), maxbtyeinbuf (IOBUFFERSIZE), btyeinbuf (0), nextstartcode (0), iseof(false)
{   
    iobitstreambuf.resize(IOBUFFERSIZE);    
}

uint32_t Annex_b::getNalu(Bitstream &nalu)
{
	std::vector<uint8_t>& nalubuf = nalu.getBitstream();
    if(nextstartcode == 0){
        while(!iseof){
			nalubuf.emplace_back(getfbtye());
            if(findStartcode(nalubuf,3)){
                nextstartcode = 3;
				nalubuf.clear();
                break;
            }
            if(findStartcode(nalubuf,2)){
                nextstartcode = 2;
				nalubuf.clear();
                break;
            }
        }       
    }

    if((nextstartcode == 0) && iseof){
        printf("cant find start code!");
		return 0;
    }
    
    while(!iseof){
		nalubuf.emplace_back(getfbtye());
        if(findStartcode(nalubuf,3)){
            nextstartcode = 3;
			//printf("startcodelength : %d szie : %d\n", nextstartcode, nalubuf.size());
			nalubuf.erase(nalubuf.end()-3-1, nalubuf.end());
            break;
        }
        if(findStartcode(nalubuf,2)){
            nextstartcode = 2;
			//printf("startcodelength : %d szie : %d\n", nextstartcode, nalubuf.size());
			nalubuf.erase(nalubuf.end()-2-1, nalubuf.end());
            break;
        }
        removeExtraByte(nalubuf);/// remove 0x03
    }
	if (iseof) {
		nextstartcode = 0;
		nalubuf.erase(nalubuf.end()- 1);
	}
	nalu.init();
    return nalubuf.size();

}

uint32_t Annex_b::getChunk()
{
    btyeinbuf = fread(&(iobitstreambuf[0]), 1, maxbtyeinbuf, bitstreamptr);

    if(btyeinbuf==0){
        iseof = true;
        return 0;
    }
    iobitstreambufiter = iobitstreambuf.begin();
    return btyeinbuf;
}

uint8_t Annex_b::getfbtye()
{
    if(btyeinbuf == 0){
        if(getChunk() == 0)
            return 0;
    }

    btyeinbuf--;
    return *iobitstreambufiter++;
}

