#ifndef _INTRA_REC_H_
#define _INTRA_REC_H_

#include "intra_base.h"

class IntraRec : public IntraBase {
public:
    IntraRec(const PictureLevelConfig &cfg);

protected:

	virtual void estimate() {};
	virtual void decidePartition() {};
    
    void reconstructure();
    void reconstructure4x4(ColourComponent compID);
    void reconstructure8x8(ColourComponent compID);
    void reconstructure16x16(ColourComponent compID);
    void reconstructureChroma();

    Macroblock recMb4x4;
    Macroblock recMb8x8;
    Macroblock recMb16x16;
};


#endif