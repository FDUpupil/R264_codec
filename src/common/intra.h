#ifndef __INTRA_H__
#define __INTRA_H__

#include <cassert>

#include "type.h"

#define IntraCheck(expr) assert(expr)

enum Intra4x4PredMode {
    INTRA_4x4_VERTICAL            = 0,
    INTRA_4x4_HORIZONTAL          = 1,
    INTRA_4x4_DC                  = 2,
    INTRA_4x4_DIAGONAL_DOWN_LEFT  = 3,
    INTRA_4x4_DIAGONAL_DOWN_RIGHT = 4,
    INTRA_4x4_VERTICAL_RIGHT      = 5,
    INTRA_4x4_HORIZONTAL_DOWN     = 6,
    INTRA_4x4_VERTICAL_LEFT       = 7,
    INTRA_4x4_HORIZONTAL_UP       = 8,
    INTRA_4x4_PRED_MODE_COUNT
};

enum Intra8x8PredMode {
    INTRA_8x8_VERTICAL            = 0,
    INTRA_8x8_HORIZONTAL          = 1,
    INTRA_8x8_DC                  = 2,
    INTRA_8x8_DIAGONAL_DOWN_LEFT  = 3,
    INTRA_8x8_DIAGONAL_DOWN_RIGHT = 4,
    INTRA_8x8_VERTICAL_RIGHT      = 5,
    INTRA_8x8_HORIZONTAL_DOWN     = 6,
    INTRA_8x8_VERTICAL_LEFT       = 7,
    INTRA_8x8_HORIZONTAL_UP       = 8,
    INTRA_8x8_PRED_MODE_COUNT
};

enum Intra16x16PredMode {
    INTRA_16x16_VERTICAL   = 0,
    INTRA_16x16_HORIZONTAL = 1,
    INTRA_16x16_DC         = 2,
    INTRA_16x16_PLANE      = 3,
    INTRA_16x16_PRED_MODE_COUNT
};

enum IntraChromaPredMode {
    INTRA_CHROMA_DC         = 0,
    INTRA_CHROMA_HORIZONTAL = 1,
    INTRA_CHROMA_VERTICAL   = 2,
    INTRA_CHROMA_PLANE      = 3,
    INTRA_CHROMA_PRED_MODE_COUNT,
    INTRA_CHROMA_420_PRED_MODE_COUNT = INTRA_CHROMA_PRED_MODE_COUNT,
    INTRA_CHROMA_422_PRED_MODE_COUNT = INTRA_CHROMA_PRED_MODE_COUNT
};

class Intra4x4Predictor {
public:
    using PredBlk = Blk<PixType, 4, 4>;

    Intra4x4Predictor(int _bitDepth);

    void setRefPixels(int _neighbour, const RefPixels &_ref);
    void predict(Intra4x4PredMode mode, PredBlk &pred) const;

private:
    int bitDepth;
    int neighbour;
    RefPixels ref;

    void predictVertical(PredBlk &pred) const;
    void predictHorizontal(PredBlk &pred) const;
    void predictDC(PredBlk &pred) const;
    void predictDiagonalDownLeft(PredBlk &pred) const;
    void predictDiagonalDownRight(PredBlk &pred) const;
    void predictVerticalRight(PredBlk &pred) const;
    void predictHorizontalDown(PredBlk &pred) const;
    void predictVerticalLeft(PredBlk &pred) const;
    void predictHorizontalUp(PredBlk &pred) const;
};

class Intra8x8Predictor {
public:
    using PredBlk = Blk<PixType, 8, 8>;

    Intra8x8Predictor(int _bitDepth);

    void setRefPixels(int _neighbour, const RefPixels &_ref);
    void predict(Intra8x8PredMode mode, PredBlk &pred) const;

private:
    int bitDepth;
    int neighbour;
    RefPixels ref;

    void filterReference(const RefPixels &src, RefPixels &dst);
    void predictVertical(PredBlk &pred) const;
    void predictHorizontal(PredBlk &pred) const;
    void predictDC(PredBlk &pred) const;
    void predictDiagonalDownLeft(PredBlk &pred) const;
    void predictDiagonalDownRight(PredBlk &pred) const;
    void predictVerticalRight(PredBlk &pred) const;
    void predictHorizontalDown(PredBlk &pred) const;
    void predictVerticalLeft(PredBlk &pred) const;
    void predictHorizontalUp(PredBlk &pred) const;
};

class Intra16x16Predictor {
public:
    using PredBlk = Blk<PixType, 16, 16>;

    Intra16x16Predictor(int _bitDepth);

    void setRefPixels(int _neighbour, const RefPixels &_ref);
    void predict(Intra16x16PredMode mode, PredBlk &pred) const;

private:
    int bitDepth;
    int neighbour;
    RefPixels ref;

    void predictVertical(PredBlk &pred) const;
    void predictHorizontal(PredBlk &pred) const;
    void predictDC(PredBlk &pred) const;
    void predictPlane(PredBlk &pred) const;
};

class IntraChromaPredictor {
public:
    using PredBlk = Blk<PixType, 16, 16>;
    
    IntraChromaPredictor(ChromaArrayType _chromaFormat, int _bitDepth);

    void setRefPixels(int _neighbour, const RefPixels &_ref);
    void predict(IntraChromaPredMode mode, PredBlk &pred) const;

private:
    ChromaArrayType chromaFormat;
    int bitDepth;
    int neighbour;
    RefPixels ref;

    void predictDC(PredBlk &pred) const;
    void predictVertical(PredBlk &pred) const;
    void predictHorizontal(PredBlk &pred) const;
    void predictPlane(PredBlk &pred) const;
};

#endif
