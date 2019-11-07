#ifndef __BLOCKY_IMAGE_H__
#define __BLOCKY_IMAGE_H__

#include "common/type.h"
#include "map_2d.h"

class BlockyImage : public Map2D<Macroblock> {
public:
    BlockyImage(ChromaArrayType _chromaFormat, uint16_t _widthInMbs, uint16_t _heightInMbs)
        : Map2D(_widthInMbs, _heightInMbs), chromaFormat(_chromaFormat) {}
    
    ChromaArrayType getChromaFormat() const { return chromaFormat; }
    
private:
    ChromaArrayType chromaFormat;
};

#endif
