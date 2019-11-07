#include "intra.h"
#include "math.h"

#define USE_4x4_REF_PIXELS_LEFT \
    PixType \
        L0 = ref.l[0], \
        L1 = ref.l[1], \
        L2 = ref.l[2], \
        L3 = ref.l[3]

#define USE_4x4_REF_PIXELS_UP_LEFT \
    PixType UL = ref.ul

#define USE_4x4_REF_PIXELS_UP \
    PixType \
        U0 = ref.u[0], \
        U1 = ref.u[1], \
        U2 = ref.u[2], \
        U3 = ref.u[3]

#define USE_4x4_REF_PIXELS_UP_RIGHT \
    PixType \
        U4 = ref.u[4], \
        U5 = ref.u[5], \
        U6 = ref.u[6], \
        U7 = ref.u[7]

#define USE_8x8_REF_PIXELS_LEFT \
    PixType \
        L0 = ref.l[0], \
        L1 = ref.l[1], \
        L2 = ref.l[2], \
        L3 = ref.l[3], \
        L4 = ref.l[4], \
        L5 = ref.l[5], \
        L6 = ref.l[6], \
        L7 = ref.l[7]

#define USE_8x8_REF_PIXELS_UP_LEFT \
    PixType UL = ref.ul

#define USE_8x8_REF_PIXELS_UP \
    PixType \
        U0 = ref.u[0], \
        U1 = ref.u[1], \
        U2 = ref.u[2], \
        U3 = ref.u[3], \
        U4 = ref.u[4], \
        U5 = ref.u[5], \
        U6 = ref.u[6], \
        U7 = ref.u[7]

#define USE_8x8_REF_PIXELS_UP_RIGHT \
    PixType \
        U8  = ref.u[8], \
        U9  = ref.u[9], \
        U10 = ref.u[10], \
        U11 = ref.u[11], \
        U12 = ref.u[12], \
        U13 = ref.u[13], \
        U14 = ref.u[14], \
        U15 = ref.u[15]

static inline PixType Filter2(PixType x, PixType y)
{
    return (x + y + 1) >> 1;
}

static inline PixType Filter3(PixType x, PixType y, PixType z)
{
    return (x + (y << 1) + z + 2) >> 2;
}

static inline void IntraPredictPlane(int width, int height, const RefPixels &ref, Blk<PixType, 16, 16> &pred, int bitDepth)
{
    int32_t a;
    int32_t b;
    int32_t c;
    int32_t H;
    int32_t V;
    uint8_t xMid;
    uint8_t yMid;

    IntraCheck(
        (width == 16 && height == 16) ||
        (width == 8  && height == 16) ||
        (width == 8  && height == 8)
    );

    xMid = (uint8_t)width  / 2;
    yMid = (uint8_t)height / 2;

    H = xMid * (ref.u[width - 1] - ref.ul);
    for (int x = 0; x < xMid - 1; ++x)
        H += (x + 1) * (ref.u[xMid + x] - ref.u[xMid - 2 - x]);
    
    V = yMid * (ref.l[height - 1] - ref.ul);
    for (int y = 0; y < yMid - 1; ++y)
        V += (y + 1) * (ref.l[yMid + y] - ref.l[yMid - 2 - y]);

    a = 16 * (ref.u[width - 1] + ref.l[height - 1]);
    b = ((width  == 16 ? 5 : 34) * H + 32) >> 6;
    c = ((height == 16 ? 5 : 34) * V + 32) >> 6;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            pred[y][x] = Clip1<int32_t>((a + b * (x - (xMid - 1)) + c * (y - (yMid - 1)) + 16) >> 5, bitDepth);
}

Intra4x4Predictor::Intra4x4Predictor(int _bitDepth)
    : bitDepth(_bitDepth)
{
}

void Intra4x4Predictor::setRefPixels(int _neighbour, const RefPixels &_ref)
{
    neighbour = _neighbour;
    ref = _ref;

    if ((neighbour & NEIGHBOUR_AVAILABLE_UP) && !(neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)) {
        neighbour |= NEIGHBOUR_AVAILABLE_UP_RIGHT;
        for (int x = 4; x < 8; ++x)
            ref.u[x] = ref.u[3];
    }
}

void Intra4x4Predictor::predict(Intra4x4PredMode mode, PredBlk &pred) const
{
    IntraCheck(mode < INTRA_4x4_PRED_MODE_COUNT);

    switch (mode) {
    case INTRA_4x4_VERTICAL:
        predictVertical(pred);
        break;

    case INTRA_4x4_HORIZONTAL:
        predictHorizontal(pred);
        break;

    case INTRA_4x4_DC:
        predictDC(pred);
        break;

    case INTRA_4x4_DIAGONAL_DOWN_LEFT:
        predictDiagonalDownLeft(pred);
        break;

    case INTRA_4x4_DIAGONAL_DOWN_RIGHT:
        predictDiagonalDownRight(pred);
        break;

    case INTRA_4x4_VERTICAL_RIGHT:
        predictVerticalRight(pred);
        break;

    case INTRA_4x4_HORIZONTAL_DOWN:
        predictHorizontalDown(pred);
        break;

    case INTRA_4x4_VERTICAL_LEFT:
        predictVerticalLeft(pred);
        break;

    case INTRA_4x4_HORIZONTAL_UP:
        predictHorizontalUp(pred);
        break;

    default:
        break;
    }
}

void Intra4x4Predictor::predictVertical(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_UP);

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            pred[y][x] = ref.u[x];
}

void Intra4x4Predictor::predictHorizontal(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            pred[y][x] = ref.l[y];
}

void Intra4x4Predictor::predictDC(PredBlk &pred) const
{
    uint32_t predVal = 0;

    if (neighbour & NEIGHBOUR_AVAILABLE_UP)
        for (int x = 0; x < 4; ++x)
            predVal += ref.u[x];
    
    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT)
        for (int y = 0; y < 4; ++y)
            predVal += ref.l[y];
    
    switch (neighbour & (NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT)) {
    case 0:
        predVal = 1 << (bitDepth - 1);
        break;

    case NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT:
        predVal = (predVal + 4) >> 3;
        break;

    default:
        predVal = (predVal + 2) >> 2;
    }

    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            pred[y][x] = (PixType)predVal;
}

void Intra4x4Predictor::predictDiagonalDownLeft(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_UP;
    USE_4x4_REF_PIXELS_UP_RIGHT;

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_UP) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)
    );
    
    p[0][0] = Filter3(U0, U1, U2); // (1, -1)
    p[0][1] = p[1][0] = Filter3(U1, U2, U3); // (2, -1)
    p[0][2] = p[1][1] = p[2][0] = Filter3(U2, U3, U4); // (3, -1)
    p[0][3] = p[1][2] = p[2][1] = p[3][0] = Filter3(U3, U4, U5); // (4, -1)
    p[1][3] = p[2][2] = p[3][1] = Filter3(U4, U5, U6); // (5, -1)
    p[2][3] = p[3][2] = Filter3(U5, U6, U7); // (6, -1)
    p[3][3] = Filter3(U6, U7, U7); // (7, -1)
}

void Intra4x4Predictor::predictDiagonalDownRight(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_LEFT;
    USE_4x4_REF_PIXELS_UP_LEFT;
    USE_4x4_REF_PIXELS_UP;

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[3][0] = Filter3(L3, L2, L1); // (-1, 2)
    p[2][0] = p[3][1] = Filter3(L2, L1, L0); // (-1, 1)
    p[1][0] = p[2][1] = p[3][2] = Filter3(L1, L0, UL); // (-1, 0)
    p[0][0] = p[1][1] = p[2][2] = p[3][3] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][1] = p[1][2] = p[2][3] = Filter3(UL, U0, U1); // (0, -1)
    p[0][2] = p[1][3] = Filter3(U0, U1, U2); // (1, -1)
    p[0][3] = Filter3(U1, U2, U3); // (2, -1)
}

void Intra4x4Predictor::predictVerticalRight(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_LEFT;
    USE_4x4_REF_PIXELS_UP_LEFT;
    USE_4x4_REF_PIXELS_UP;
    Unused(L3);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[3][0] = Filter3(L2, L1, L0); // (-1, 1)
    p[2][0] = Filter3(L1, L0, UL); // (-1, 0)
    p[1][0] = p[3][1] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][0] = p[2][1] = Filter2(UL, U0); // (-0.5, -1)
    p[1][1] = p[3][2] = Filter3(UL, U0, U1); // (0, -1)
    p[0][1] = p[2][2] = Filter2(U0, U1); // (0.5, -1)
    p[1][2] = p[3][3] = Filter3(U0, U1, U2); // (1, -1)
    p[0][2] = p[2][3] = Filter2(U1, U2); // (1.5, -1)
    p[1][3] = Filter3(U1, U2, U3); // (2, -1)
    p[0][3] = Filter2(U2, U3); // (2.5, -1)
}

void Intra4x4Predictor::predictHorizontalDown(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_LEFT;
    USE_4x4_REF_PIXELS_UP_LEFT;
    USE_4x4_REF_PIXELS_UP;
    Unused(U3);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[3][0] = Filter2(L3, L2); // (-1, 2.5)
    p[3][1] = Filter3(L3, L2, L1); // (-1, 2)
    p[2][0] = p[3][2] = Filter2(L2, L1); // (-1, 1.5)
    p[2][1] = p[3][3] = Filter3(L2, L1, L0); // (-1, 1)
    p[1][0] = p[2][2] = Filter2(L1, L0); // (-1, 0.5)
    p[1][1] = p[2][3] = Filter3(L1, L0, UL); // (-1, 0)
    p[0][0] = p[1][2] = Filter2(L0, UL); // (-1, -0.5)
    p[0][1] = p[1][3] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][2] = Filter3(UL, U0, U1); // (0, -1)
    p[0][3] = Filter3(U0, U1, U2); // (1, -1)
}

void Intra4x4Predictor::predictVerticalLeft(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_UP;
    USE_4x4_REF_PIXELS_UP_RIGHT;
    Unused(U7);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_UP) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)
    );

    p[0][0] = Filter2(U0, U1); // (0.5, -1)
    p[1][0] = Filter3(U0, U1, U2); // (1, -1)
    p[0][1] = p[2][0] = Filter2(U1, U2); // (1.5, -1)
    p[1][1] = p[3][0] = Filter3(U1, U2, U3); // (2, -1)
    p[0][2] = p[2][1] = Filter2(U2, U3); // (2.5, -1)
    p[1][2] = p[3][1] = Filter3(U2, U3, U4); // (3, -1)
    p[0][3] = p[2][2] = Filter2(U3, U4); // (3.5, -1)
    p[1][3] = p[3][2] = Filter3(U3, U4, U5); // (4, -1)
    p[2][3] = Filter2(U4, U5); // (4.5, -1)
    p[3][3] = Filter3(U4, U5, U6); // (5, -1)
}

void Intra4x4Predictor::predictHorizontalUp(PredBlk &p) const
{
    USE_4x4_REF_PIXELS_LEFT;

    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);

    p[3][3] = L3; // (-1, 5)
    p[3][2] = L3; // (-1, 4.5)
    p[3][1] = p[2][3] = L3; // (-1, 4)
    p[3][0] = p[2][2] = L3; // (-1, 3.5)
    p[2][1] = p[1][3] = Filter3(L3, L3, L2); // (-1, 3)
    p[2][0] = p[1][2] = Filter2(L3, L2); // (-1, 2.5)
    p[1][1] = p[0][3] = Filter3(L3, L2, L1); // (-1, 2)
    p[1][0] = p[0][2] = Filter2(L2, L1); // (-1, 1.5)
    p[0][1] = Filter3(L2, L1, L0); // (-1, 1)
    p[0][0] = Filter2(L1, L0); // (-1, 0.5)
}

Intra8x8Predictor::Intra8x8Predictor(int _bitDepth)
    : bitDepth(_bitDepth)
{
}

void Intra8x8Predictor::setRefPixels(int _neighbour, const RefPixels &_ref)
{
    neighbour = _neighbour;
    filterReference(_ref, ref);

    if ((neighbour & NEIGHBOUR_AVAILABLE_UP) && !(neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)) {
        RefPixels filled;
        neighbour |= NEIGHBOUR_AVAILABLE_UP_RIGHT;
        filled = _ref;
        for (int x = 8; x < 16; ++x)
            filled.u[x] = filled.u[7];
        filterReference(filled, ref);
    } else {
        filterReference(_ref, ref);
    }
}

void Intra8x8Predictor::predict(Intra8x8PredMode mode, PredBlk &pred) const
{
    IntraCheck(mode < INTRA_8x8_PRED_MODE_COUNT);

    switch (mode) {
    case INTRA_8x8_VERTICAL:
        predictVertical(pred);
        break;

    case INTRA_8x8_HORIZONTAL:
        predictHorizontal(pred);
        break;

    case INTRA_8x8_DC:
        predictDC(pred);
        break;

    case INTRA_8x8_DIAGONAL_DOWN_LEFT:
        predictDiagonalDownLeft(pred);
        break;

    case INTRA_8x8_DIAGONAL_DOWN_RIGHT:
        predictDiagonalDownRight(pred);
        break;

    case INTRA_8x8_VERTICAL_RIGHT:
        predictVerticalRight(pred);
        break;

    case INTRA_8x8_HORIZONTAL_DOWN:
        predictHorizontalDown(pred);
        break;

    case INTRA_8x8_VERTICAL_LEFT:
        predictVerticalLeft(pred);
        break;

    case INTRA_8x8_HORIZONTAL_UP:
        predictHorizontalUp(pred);
        break;

    default:
        break;
    }
}

void Intra8x8Predictor::filterReference(const RefPixels &src, RefPixels &dst)
{
    switch (neighbour & (NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT)) {
    case NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT:
        dst.ul = Filter3(src.l[0], src.ul, src.u[0]);
        break;

    case NEIGHBOUR_AVAILABLE_UP:
        dst.ul = Filter3(src.ul, src.ul, src.u[0]);
        break;

    case NEIGHBOUR_AVAILABLE_LEFT:
        dst.ul = Filter3(src.ul, src.ul, src.l[0]);
        break;

    default:
        return;
    }

    if (neighbour & NEIGHBOUR_AVAILABLE_UP) {
        dst.u[0] = Filter3((neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) ? src.ul : src.u[0], src.u[0], src.u[1]);
        for (int x = 1; x < 15; ++x)
            dst.u[x] = Filter3(src.u[x - 1], src.u[x], src.u[x + 1]);
        dst.u[15] = Filter3(src.u[14], src.u[15], src.u[15]);
    }

    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT) {
        dst.l[0] = Filter3((neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) ? src.ul : src.l[0], src.l[0], src.l[1]);
        for (int y = 1; y < 7; ++y)
            dst.l[y] = Filter3(src.l[y - 1], src.l[y], src.l[y + 1]);
        dst.l[7] = Filter3(src.l[6], src.l[7], src.l[7]);
    }
}

void Intra8x8Predictor::predictVertical(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_UP);

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            pred[y][x] = ref.u[x];
}

void Intra8x8Predictor::predictHorizontal(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            pred[y][x] = ref.l[y];
}

void Intra8x8Predictor::predictDC(PredBlk &pred) const
{
    uint32_t predVal = 0;

    if (neighbour & NEIGHBOUR_AVAILABLE_UP)
        for (int x = 0; x < 8; ++x)
            predVal += ref.u[x];
    
    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT)
        for (int y = 0; y < 8; ++y)
            predVal += ref.l[y];
    
    switch (neighbour & (NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT)) {
    case 0:
        predVal = 1 << (bitDepth - 1);
        break;

    case NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT:
        predVal = (predVal + 8) >> 4;
        break;

    default:
        predVal = (predVal + 4) >> 3;
    }

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            pred[y][x] = (PixType)predVal;
}

void Intra8x8Predictor::predictDiagonalDownLeft(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_UP;
    USE_8x8_REF_PIXELS_UP_RIGHT;

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_UP) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)
    );
    
    p[0][0] = Filter3(U0, U1, U2); // (1, -1)
    p[0][1] = p[1][0] = Filter3(U1, U2, U3); // (2, -1)
    p[0][2] = p[1][1] = p[2][0] = Filter3(U2, U3, U4); // (3, -1)
    p[0][3] = p[1][2] = p[2][1] = p[3][0] = Filter3(U3, U4, U5); // (4, -1)
    p[0][4] = p[1][3] = p[2][2] = p[3][1] = p[4][0] = Filter3(U4, U5, U6); // (5, -1)
    p[0][5] = p[1][4] = p[2][3] = p[3][2] = p[4][1] = p[5][0] = Filter3(U5, U6, U7); // (6, -1)
    p[0][6] = p[1][5] = p[2][4] = p[3][3] = p[4][2] = p[5][1] = p[6][0] = Filter3(U6, U7, U8); // (7, -1)
    p[0][7] = p[1][6] = p[2][5] = p[3][4] = p[4][3] = p[5][2] = p[6][1] = p[7][0] = Filter3(U7, U8, U9); // (8, -1)
    p[1][7] = p[2][6] = p[3][5] = p[4][4] = p[5][3] = p[6][2] = p[7][1] = Filter3(U8, U9, U10); // (9, -1)
    p[2][7] = p[3][6] = p[4][5] = p[5][4] = p[6][3] = p[7][2] = Filter3(U9, U10, U11); // (10, -1)
    p[3][7] = p[4][6] = p[5][5] = p[6][4] = p[7][3] = Filter3(U10, U11, U12); // (11, -1)
    p[4][7] = p[5][6] = p[6][5] = p[7][4] = Filter3(U11, U12, U13); // (12, -1)
    p[5][7] = p[6][6] = p[7][5] = Filter3(U12, U13, U14); // (13, -1)
    p[6][7] = p[7][6] = Filter3(U13, U14, U15); // (14, -1)
    p[7][7] = Filter3(U14, U15, U15); // (15, -1)
}

void Intra8x8Predictor::predictDiagonalDownRight(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_LEFT;
    USE_8x8_REF_PIXELS_UP_LEFT;
    USE_8x8_REF_PIXELS_UP;

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[7][0] = Filter3(L7, L6, L5); // (-1, 6)
    p[6][0] = p[7][1] = Filter3(L6, L5, L4); // (-1, 5)
    p[5][0] = p[6][1] = p[7][2] = Filter3(L5, L4, L3); // (-1, 4)
    p[4][0] = p[5][1] = p[6][2] = p[7][3] = Filter3(L4, L3, L2); // (-1, 3)
    p[3][0] = p[4][1] = p[5][2] = p[6][3] = p[7][4] = Filter3(L3, L2, L1); // (-1, 2)
    p[2][0] = p[3][1] = p[4][2] = p[5][3] = p[6][4] = p[7][5] = Filter3(L2, L1, L0); // (-1, 1)
    p[1][0] = p[2][1] = p[3][2] = p[4][3] = p[5][4] = p[6][5] = p[7][6] = Filter3(L1, L0, UL); // (-1, 0)
    p[0][0] = p[1][1] = p[2][2] = p[3][3] = p[4][4] = p[5][5] = p[6][6] = p[7][7] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][1] = p[1][2] = p[2][3] = p[3][4] = p[4][5] = p[5][6] = p[6][7] = Filter3(UL, U0, U1); // (0, -1)
    p[0][2] = p[1][3] = p[2][4] = p[3][5] = p[4][6] = p[5][7] = Filter3(U0, U1, U2); // (1, -1)
    p[0][3] = p[1][4] = p[2][5] = p[3][6] = p[4][7] = Filter3(U1, U2, U3); // (2, -1)
    p[0][4] = p[1][5] = p[2][6] = p[3][7] = Filter3(U2, U3, U4); // (3, -1)
    p[0][5] = p[1][6] = p[2][7] = Filter3(U3, U4, U5); // (4, -1)
    p[0][6] = p[1][7] = Filter3(U4, U5, U6); // (5, -1)
    p[0][7] = Filter3(U5, U6, U7); // (6, -1)
}

void Intra8x8Predictor::predictVerticalRight(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_LEFT;
    USE_8x8_REF_PIXELS_UP_LEFT;
    USE_8x8_REF_PIXELS_UP;
    Unused(L7);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[7][0] = Filter3(L6, L5, L4); // (-1, 5)
    p[6][0] = Filter3(L5, L4, L3); // (-1, 4)
    p[5][0] = p[7][1] = Filter3(L4, L3, L2); // (-1, 3)
    p[4][0] = p[6][1] = Filter3(L3, L2, L1); // (-1, 2)
    p[3][0] = p[5][1] = p[7][2] = Filter3(L2, L1, L0); // (-1, 1)
    p[2][0] = p[4][1] = p[6][2] = Filter3(L1, L0, UL); // (-1, 0)
    p[1][0] = p[3][1] = p[5][2] = p[7][3] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][0] = p[2][1] = p[4][2] = p[6][3] = Filter2(UL, U0); // (-0.5, -1)
    p[1][1] = p[3][2] = p[5][3] = p[7][4] = Filter3(UL, U0, U1); // (0, -1)
    p[0][1] = p[2][2] = p[4][3] = p[6][4] = Filter2(U0, U1); // (0.5, -1)
    p[1][2] = p[3][3] = p[5][4] = p[7][5] = Filter3(U0, U1, U2); // (1, -1)
    p[0][2] = p[2][3] = p[4][4] = p[6][5] = Filter2(U1, U2); // (1.5, -1)
    p[1][3] = p[3][4] = p[5][5] = p[7][6] = Filter3(U1, U2, U3); // (2, -1)
    p[0][3] = p[2][4] = p[4][5] = p[6][6] = Filter2(U2, U3); // (2.5, -1)
    p[1][4] = p[3][5] = p[5][6] = p[7][7] = Filter3(U2, U3, U4); // (3, -1)
    p[0][4] = p[2][5] = p[4][6] = p[6][7] = Filter2(U3, U4); // (3.5, -1)
    p[1][5] = p[3][6] = p[5][7] = Filter3(U3, U4, U5); // (4, -1)
    p[0][5] = p[2][6] = p[4][7] = Filter2(U4, U5); // (4.5, -1)
    p[1][6] = p[3][7] = Filter3(U4, U5, U6); // (5, -1)
    p[0][6] = p[2][7] = Filter2(U5, U6); // (5.5, -1)
    p[1][7] = Filter3(U5, U6, U7); // (6, -1)
    p[0][7] = Filter2(U6, U7); // (6.5, -1)
}

void Intra8x8Predictor::predictHorizontalDown(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_LEFT;
    USE_8x8_REF_PIXELS_UP_LEFT;
    USE_8x8_REF_PIXELS_UP;
    Unused(U7);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    p[7][0] = Filter2(L7, L6); // (-1, 6.5)
    p[7][1] = Filter3(L7, L6, L5); // (-1, 6)
    p[6][0] = p[7][2] = Filter2(L6, L5); // (-1, 5.5)
    p[6][1] = p[7][3] = Filter3(L6, L5, L4); // (-1, 5)
    p[5][0] = p[6][2] = p[7][4] = Filter2(L5, L4); // (-1, 4.5)
    p[5][1] = p[6][3] = p[7][5] = Filter3(L5, L4, L3); // (-1, 4)
    p[4][0] = p[5][2] = p[6][4] = p[7][6] = Filter2(L4, L3); // (-1, 3.5)
    p[4][1] = p[5][3] = p[6][5] = p[7][7] = Filter3(L4, L3, L2); // (-1, 3)
    p[3][0] = p[4][2] = p[5][4] = p[6][6] = Filter2(L3, L2); // (-1, 2.5)
    p[3][1] = p[4][3] = p[5][5] = p[6][7] = Filter3(L3, L2, L1); // (-1, 2)
    p[2][0] = p[3][2] = p[4][4] = p[5][6] = Filter2(L2, L1); // (-1, 1.5)
    p[2][1] = p[3][3] = p[4][5] = p[5][7] = Filter3(L2, L1, L0); // (-1, 1)
    p[1][0] = p[2][2] = p[3][4] = p[4][6] = Filter2(L1, L0); // (-1, 0.5)
    p[1][1] = p[2][3] = p[3][5] = p[4][7] = Filter3(L1, L0, UL); // (-1, 0)
    p[0][0] = p[1][2] = p[2][4] = p[3][6] = Filter2(L0, UL); // (-1, -0.5)
    p[0][1] = p[1][3] = p[2][5] = p[3][7] = Filter3(L0, UL, U0); // (-1, -1)
    p[0][2] = p[1][4] = p[2][6] = Filter3(UL, U0, U1); // (0, -1)
    p[0][3] = p[1][5] = p[2][7] = Filter3(U0, U1, U2); // (1, -1)
    p[0][4] = p[1][6] = Filter3(U1, U2, U3); // (2, -1)
    p[0][5] = p[1][7] = Filter3(U2, U3, U4); // (3, -1)
    p[0][6] = Filter3(U3, U4, U5); // (4, -1)
    p[0][7] = Filter3(U4, U5, U6); // (5, -1)
}

void Intra8x8Predictor::predictVerticalLeft(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_UP;
    USE_8x8_REF_PIXELS_UP_RIGHT;
    Unused(U13);
    Unused(U14);
    Unused(U15);

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_UP) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_RIGHT)
    );

    p[0][0] = Filter2(U0, U1); // (0.5, -1)
    p[1][0] = Filter3(U0, U1, U2); // (1, -1)
    p[0][1] = p[2][0] = Filter2(U1, U2); // (1.5, -1)
    p[1][1] = p[3][0] = Filter3(U1, U2, U3); // (2, -1)
    p[0][2] = p[2][1] = p[4][0] = Filter2(U2, U3); // (2.5, -1)
    p[1][2] = p[3][1] = p[5][0] = Filter3(U2, U3, U4); // (3, -1)
    p[0][3] = p[2][2] = p[4][1] = p[6][0] = Filter2(U3, U4); // (3.5, -1)
    p[1][3] = p[3][2] = p[5][1] = p[7][0] = Filter3(U3, U4, U5); // (4, -1)
    p[0][4] = p[2][3] = p[4][2] = p[6][1] = Filter2(U4, U5); // (4.5, -1)
    p[1][4] = p[3][3] = p[5][2] = p[7][1] = Filter3(U4, U5, U6); // (5, -1)
    p[0][5] = p[2][4] = p[4][3] = p[6][2] = Filter2(U5, U6); // (5.5, -1)
    p[1][5] = p[3][4] = p[5][3] = p[7][2] = Filter3(U5, U6, U7); // (6, -1)
    p[0][6] = p[2][5] = p[4][4] = p[6][3] = Filter2(U6, U7); // (6.5, -1)
    p[1][6] = p[3][5] = p[5][4] = p[7][3] = Filter3(U6, U7, U8); // (7, -1)
    p[0][7] = p[2][6] = p[4][5] = p[6][4] = Filter2(U7, U8); // (7.5, -1)
    p[1][7] = p[3][6] = p[5][5] = p[7][4] = Filter3(U7, U8, U9); // (8, -1)
    p[2][7] = p[4][6] = p[6][5] = Filter2(U8, U9); // (8.5, -1)
    p[3][7] = p[5][6] = p[7][5] = Filter3(U8, U9, U10); // (9, -1)
    p[4][7] = p[6][6] = Filter2(U9, U10); // (9.5, -1)
    p[5][7] = p[7][6] = Filter3(U9, U10, U11); // (10, -1)
    p[6][7] = Filter2(U10, U11); // (10.5, -1)
    p[7][7] = Filter3(U10, U11, U12); // (11, -1)
}

void Intra8x8Predictor::predictHorizontalUp(PredBlk &p) const
{
    USE_8x8_REF_PIXELS_LEFT;

    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);

    p[7][7] = L7; // (-1, 11)
    p[7][6] = L7; // (-1, 10.5)
    p[7][5] = p[6][7] = L7; // (-1, 10)
    p[7][4] = p[6][6] = L7; // (-1, 9.5)
    p[7][3] = p[6][5] = p[5][7] = L7; // (-1, 9)
    p[7][2] = p[6][4] = p[5][6] = L7; // (-1, 8.5)
    p[7][1] = p[6][3] = p[5][5] = p[4][7] = L7; // (-1, 8)
    p[7][0] = p[6][2] = p[5][4] = p[4][6] = L7; // (-1, 7.5)
    p[6][1] = p[5][3] = p[4][5] = p[3][7] = Filter3(L7, L7, L6); // (-1, 7)
    p[6][0] = p[5][2] = p[4][4] = p[3][6] = Filter2(L7, L6); // (-1, 6.5)
    p[5][1] = p[4][3] = p[3][5] = p[2][7] = Filter3(L7, L6, L5); // (-1, 6)
    p[5][0] = p[4][2] = p[3][4] = p[2][6] = Filter2(L6, L5); // (-1, 5.5)
    p[4][1] = p[3][3] = p[2][5] = p[1][7] = Filter3(L6, L5, L4); // (-1, 5)
    p[4][0] = p[3][2] = p[2][4] = p[1][6] = Filter2(L5, L4); // (-1, 4.5)
    p[3][1] = p[2][3] = p[1][5] = p[0][7] = Filter3(L5, L4, L3); // (-1, 4)
    p[3][0] = p[2][2] = p[1][4] = p[0][6] = Filter2(L4, L3); // (-1, 3.5)
    p[2][1] = p[1][3] = p[0][5] = Filter3(L4, L3, L2); // (-1, 3)
    p[2][0] = p[1][2] = p[0][4] = Filter2(L3, L2); // (-1, 2.5)
    p[1][1] = p[0][3] = Filter3(L3, L2, L1); // (-1, 2)
    p[1][0] = p[0][2] = Filter2(L2, L1); // (-1, 1.5)
    p[0][1] = Filter3(L2, L1, L0); // (-1, 1)
    p[0][0] = Filter2(L1, L0); // (-1, 0.5)
}

Intra16x16Predictor::Intra16x16Predictor(int _bitDepth)
    : bitDepth(_bitDepth)
{
}

void Intra16x16Predictor::setRefPixels(int _neighbour, const RefPixels &_ref)
{
    neighbour = _neighbour;
    ref = _ref;
}

void Intra16x16Predictor::predict(Intra16x16PredMode mode, PredBlk &pred) const
{
    IntraCheck(mode < INTRA_16x16_PRED_MODE_COUNT);

    switch (mode) {
    case INTRA_16x16_VERTICAL:
        predictVertical(pred);
        break;

    case INTRA_16x16_HORIZONTAL:
        predictHorizontal(pred);
        break;

    case INTRA_16x16_DC:
        predictDC(pred);
        break;

    case INTRA_16x16_PLANE:
        predictPlane(pred);
        break;

    default:
        break;
    }
}

void Intra16x16Predictor::predictVertical(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_UP);

    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            pred[y][x] = ref.u[x];
}

void Intra16x16Predictor::predictHorizontal(PredBlk &pred) const
{
    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);

    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            pred[y][x] = ref.l[y];
}

void Intra16x16Predictor::predictDC(PredBlk &pred) const
{
    uint32_t predVal = 0;

    if (neighbour & NEIGHBOUR_AVAILABLE_UP)
        for (int x = 0; x < 16; ++x)
            predVal += ref.u[x];
    
    if (neighbour & NEIGHBOUR_AVAILABLE_LEFT)
        for (int y = 0; y < 16; ++y)
            predVal += ref.l[y];
    
    switch (neighbour & (NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT)) {
    case 0:
        predVal = 1 << (bitDepth - 1);
        break;

    case NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT:
        predVal = (predVal + 16) >> 5;
        break;

    default:
        predVal = (predVal + 8) >> 4;
    }

    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            pred[y][x] = (PixType)predVal;
}

void Intra16x16Predictor::predictPlane(PredBlk &pred) const
{
    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    IntraPredictPlane(16, 16, ref, pred, bitDepth);
}

IntraChromaPredictor::IntraChromaPredictor(ChromaArrayType _chromaFormat, int _bitDepth)
    : chromaFormat(_chromaFormat), bitDepth(_bitDepth)
{
    IntraCheck(
        (chromaFormat == CHROMA_FORMAT_420) ||
        (chromaFormat == CHROMA_FORMAT_422)
    );
}

void IntraChromaPredictor::setRefPixels(int _neighbour, const RefPixels &_ref)
{
    neighbour = _neighbour;
    ref = _ref;
}

void IntraChromaPredictor::predict(IntraChromaPredMode mode, PredBlk &pred) const
{
    IntraCheck(mode < INTRA_CHROMA_PRED_MODE_COUNT);

    switch (mode) {
    case INTRA_CHROMA_DC:
        predictDC(pred);
        break;
        
    case INTRA_CHROMA_HORIZONTAL:
        predictHorizontal(pred);
        break;

    case INTRA_CHROMA_VERTICAL:
        predictVertical(pred);
        break;

    case INTRA_CHROMA_PLANE:
        predictPlane(pred);
        break;

    default:
        break;
    }
}

void IntraChromaPredictor::predictDC(PredBlk &pred) const
{
    uint32_t predVal;
    int height;
    
    height = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

    for (int yInBlks = 0; yInBlks < height / 4; ++yInBlks) {
        int yO = yInBlks * 4;

        for (int xInBlks = 0; xInBlks < 8 / 4; ++xInBlks) {
            int xO = xInBlks * 4;
            int nb = neighbour;

            if (xInBlks == 0 && yInBlks != 0) {
                // If left block is available, then up block is not used
                if (nb & NEIGHBOUR_AVAILABLE_LEFT)
                    nb &= ~NEIGHBOUR_AVAILABLE_UP;
            } else if (xInBlks != 0 && yInBlks == 0) {
                // If up block is available, then left block is not used
                if (nb & NEIGHBOUR_AVAILABLE_UP)
                    nb &= ~NEIGHBOUR_AVAILABLE_LEFT;
            }

            predVal = 0;

            if (nb & NEIGHBOUR_AVAILABLE_UP)
                for (int x = xO; x < xO + 4; ++x)
                    predVal += ref.u[x];
            
            if (nb & NEIGHBOUR_AVAILABLE_LEFT)
                for (int y = yO; y < yO + 4; ++y)
                    predVal += ref.l[y];
            
            switch (nb & (NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT)) {
            case 0:
                predVal = 1 << (bitDepth - 1);
                break;

            case NEIGHBOUR_AVAILABLE_UP | NEIGHBOUR_AVAILABLE_LEFT:
                predVal = (predVal + 4) >> 3;
                break;

            default:
                predVal = (predVal + 2) >> 2;
            }

            for (int y = yO; y < yO + 4; ++y)
                for (int x = xO; x < xO + 4; ++x)
                    pred[y][x] = (PixType)predVal;
        }
    }
}

void IntraChromaPredictor::predictVertical(PredBlk &pred) const
{
    int height;

    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_UP);
    
    height = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < 8; ++x)
            pred[y][x] = ref.u[x];
}

void IntraChromaPredictor::predictHorizontal(PredBlk &pred) const
{
    int height;

    IntraCheck(neighbour & NEIGHBOUR_AVAILABLE_LEFT);
    
    height = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < 8; ++x)
            pred[y][x] = ref.l[y];
}

void IntraChromaPredictor::predictPlane(PredBlk &pred) const
{
    int height;

    IntraCheck(
        (neighbour & NEIGHBOUR_AVAILABLE_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP_LEFT) &&
        (neighbour & NEIGHBOUR_AVAILABLE_UP)
    );

    height = (chromaFormat == CHROMA_FORMAT_420) ? 8 : 16;

    IntraPredictPlane(8, height, ref, pred, bitDepth);
}
