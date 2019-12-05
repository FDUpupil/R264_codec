#ifndef __BLOCK_H__
#define __BLOCK_H__

#include"type.h"
#include"math.h"

const Coordinate ZigZag2x2[4] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }
};

const Coordinate ZigZag2x4[8] = {
    { 0, 0 }, { 0, 1 }, { 1, 0 }, { 0, 2 },
    { 0, 3 }, { 1, 1 }, { 1, 2 }, { 1, 3 }
};

const Coordinate ZigZag4x4[16] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 0, 2 },
    { 1, 1 }, { 2, 0 }, { 3, 0 }, { 2, 1 },
    { 1, 2 }, { 0, 3 }, { 1, 3 }, { 2, 2 },
    { 3, 1 }, { 3, 2 }, { 2, 3 }, { 3, 3 }
};

const Coordinate ZigZag8x8[64] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 0, 2 },
    { 1, 1 }, { 2, 0 }, { 3, 0 }, { 2, 1 },
    { 1, 2 }, { 0, 3 }, { 0, 4 }, { 1, 3 },
    { 2, 2 }, { 3, 1 }, { 4, 0 }, { 5, 0 },
    { 4, 1 }, { 3, 2 }, { 2, 3 }, { 1, 4 },
    { 0, 5 }, { 0, 6 }, { 1, 5 }, { 2, 4 },
    { 3, 3 }, { 4, 2 }, { 5, 1 }, { 6, 0 },
    { 7, 0 }, { 6, 1 }, { 5, 2 }, { 4, 3 },
    { 3, 4 }, { 2, 5 }, { 1, 6 }, { 0, 7 },
    { 1, 7 }, { 2, 6 }, { 3, 5 }, { 4, 4 },
    { 5, 3 }, { 6, 2 }, { 7, 1 }, { 7, 2 },
    { 6, 3 }, { 5, 4 }, { 4, 5 }, { 3, 6 },
    { 2, 7 }, { 3, 7 }, { 4, 6 }, { 5, 5 },
    { 6, 4 }, { 7, 3 }, { 7, 4 }, { 6, 5 },
    { 5, 6 }, { 4, 7 }, { 5, 7 }, { 6, 6 },
    { 7, 5 }, { 7, 6 }, { 6, 7 }, { 7, 7 }
};

template <int width, int height, typename T1, typename T2>
inline void CopyBlock(const Blk<T1, width, height> &src, Blk<T2, width, height> &dst)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            dst[y][x] = src[y][x];
}

template <int width, int height, typename T1, typename T2>
inline void DiffBlock(const Blk<T1, width, height> &org, const Blk<T1, width, height> &pred, Blk<T2, width, height> &res)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            res[y][x] = T2(org[y][x] - pred[y][x]);
}

// template <int width, int height, typename T1, typename T2>
// inline void SumBlock(const Blk<T1, width, height> &pred, const Blk<T2, width, height> &res, Blk<T1, width, height> &org)
// {
//     for (int y = 0; y < height; ++y)
//         for (int x = 0; x < width; ++x)
//             org[y][x] = T1(T2(pred[y][x]) + res[y][x]);
// }

template <int width, int height, typename T1, typename T2>
inline void RestoreBlock(const Blk<T1, width, height> &pred, const Blk<T2, width, height> &res, Blk<T1, width, height> &rec, uint8_t bitDepth)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            rec[y][x] = T1(Clip1(pred[y][x] + res[y][x], bitDepth));
}

template <int mbW, int mbH, int sbW, int sbH, typename T>
inline void SplitBlock(const Blk<T, 16, 16> &src, Blk<T, sbW, sbH>(&dst)[mbH / sbH][mbW / sbW])
{
    for (int yInSbs = 0; yInSbs < mbH / sbH; ++yInSbs)
        for (int xInSbs = 0; xInSbs < mbW / sbW; ++xInSbs)
            for (int yOfSbs = 0; yOfSbs < sbH; ++yOfSbs)
                for (int xOfSbs = 0; xOfSbs < sbW; ++xOfSbs)
                    dst[yInSbs][xInSbs][yOfSbs][xOfSbs] = src[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs];
}

template <int mbW, int mbH, int sbW, int sbH, typename T1, typename T2>
inline void SplitBlock(const Blk<T1, 16, 16> &org, const Blk<T1, 16, 16> &pred, Blk<T2, sbW, sbH>(&res)[mbH / sbH][mbW / sbW])
{
    for (int yInSbs = 0; yInSbs < mbH / sbH; ++yInSbs)
        for (int xInSbs = 0; xInSbs < mbW / sbW; ++xInSbs)
            for (int yOfSbs = 0; yOfSbs < sbH; ++yOfSbs)
                for (int xOfSbs = 0; xOfSbs < sbW; ++xOfSbs)
                    res[yInSbs][xInSbs][yOfSbs][xOfSbs] = org[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs] - pred[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs];
}

template <int mbW, int mbH, int sbW, int sbH, typename T>
inline void SpliceBlock(const Blk<T, sbW, sbH>(&src)[mbH / sbH][mbW / sbW], Blk<T, 16, 16> &dst)
{
    for (int yInSbs = 0; yInSbs < mbH / sbH; ++yInSbs)
        for (int xInSbs = 0; xInSbs < mbW / sbW; ++xInSbs)
            for (int yOfSbs = 0; yOfSbs < sbH; ++yOfSbs)
                for (int xOfSbs = 0; xOfSbs < sbW; ++xOfSbs)
                    dst[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs] = src[yInSbs][xInSbs][yOfSbs][xOfSbs];
}

template <int mbW, int mbH, int sbW, int sbH, typename T1, typename T2>
inline void SpliceBlock(const Blk<T1, 16, 16> &pred, const Blk<T2, sbW, sbH>(&res)[mbH / sbH][mbW / sbW], Blk<T1, 16, 16> &rec, uint8_t bitDepth)
{
    for (int yInSbs = 0; yInSbs < mbH / sbH; ++yInSbs)
        for (int xInSbs = 0; xInSbs < mbW / sbW; ++xInSbs)
            for (int yOfSbs = 0; yOfSbs < sbH; ++yOfSbs)
                for (int xOfSbs = 0; xOfSbs < sbW; ++xOfSbs)
                    rec[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs] = T1(Clip1(pred[yInSbs * sbH + yOfSbs][xInSbs * sbW + xOfSbs] + res[yInSbs][xInSbs][yOfSbs][xOfSbs], bitDepth));
}

template <typename T>
inline void ScanZigZag2x2(const Blk<T, 2, 2> &blk, T *seq, uint8_t startIdx = 0, uint8_t endIdx = 3)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        *seq++ = blk[ZigZag2x2[i].y][ZigZag2x2[i].x];
}

template <typename T>
inline void ScanZigZag2x4(const Blk<T, 2, 4> &blk, T *seq, uint8_t startIdx = 0, uint8_t endIdx = 7)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        *seq++ = blk[ZigZag2x4[i].y][ZigZag2x4[i].x];
}

template <typename T>
inline void ScanZigZag4x4(const Blk<T, 4, 4> &blk, T *seq, uint8_t startIdx = 0, uint8_t endIdx = 15)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        *seq++ = blk[ZigZag4x4[i].y][ZigZag4x4[i].x];
}

template <typename T>
inline void ScanZigZag8x8(const Blk<T, 8, 8> &blk, T *seq, uint8_t startIdx = 0, uint8_t endIdx = 63)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        *seq++ = blk[ZigZag8x8[i].y][ZigZag8x8[i].x];
}

template <typename T>
inline void ScanZigZag8x8(T* seq, Blk<T, 8, 8> & blk, uint8_t startIdx = 0, uint8_t endIdx = 63)
{
	for (uint8_t i = startIdx; i <= endIdx; ++i)
		blk[ZigZag8x8[i].y][ZigZag8x8[i].x] = * seq++;
}

template <typename T>
inline void InvScanZigZag2x2(const T *seq, Blk<T, 2, 2> &blk, uint8_t startIdx = 0, uint8_t endIdx = 3)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        blk[ZigZag2x2[i].y][ZigZag2x2[i].x] = *seq++;
}

template <typename T>
inline void InvScanZigZag2x4(const T *seq, Blk<T, 2, 4> &blk, uint8_t startIdx = 0, uint8_t endIdx = 7)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        blk[ZigZag2x4[i].y][ZigZag2x4[i].x] = *seq++;
}

template <typename T>
inline void InvScanZigZag4x4(const T *seq, Blk<T, 4, 4> &blk, uint8_t startIdx = 0, uint8_t endIdx = 15)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        blk[ZigZag4x4[i].y][ZigZag4x4[i].x] = *seq++;
}

template <typename T>
inline void InvScanZigZag8x8(const T *seq, Blk<T, 8, 8> &blk, uint8_t startIdx = 0, uint8_t endIdx = 63)
{
    for (uint8_t i = startIdx; i <= endIdx; ++i)
        blk[ZigZag8x8[i].y][ZigZag8x8[i].x] = *seq++;
}

const Blk<uint8_t, 2, 2> MacroblockScan2x2 = {
    { 0, 1 },
    { 2, 3 }
};

const Blk<uint8_t, 2, 4> MacroblockScan2x4 = {
    { 0, 1 },
    { 2, 3 },
    { 4, 5 },
    { 6, 7 }
};

const Blk<uint8_t, 4, 4> MacroblockScan4x4 = {
    {  0,  1,  4,  5 },
    {  2,  3,  6,  7 },
    {  8,  9, 12, 13 },
    { 10, 11, 14, 15 }
};

const Coordinate MacroblockInvScan2x2[4] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 }
};

const Coordinate MacroblockInvScan2x4[8] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
    { 0, 2 }, { 1, 2 }, { 0, 3 }, { 1, 3 }
};

const Coordinate MacroblockInvScan4x4[16] = {
    { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 },
    { 2, 0 }, { 3, 0 }, { 2, 1 }, { 3, 1 },
    { 0, 2 }, { 1, 2 }, { 0, 3 }, { 1, 3 },
    { 2, 2 }, { 3, 2 }, { 2, 3 }, { 3, 3 }
};

template <int width, int height, typename T>
inline bool checkIfNonZero(const Blk<T, width, height> &blk)
{
    for (uint8_t yOfBlks = 0; yOfBlks < height; ++yOfBlks)
        for (uint8_t xOfBlks = 0; xOfBlks < width; ++xOfBlks)
            if (blk[yOfBlks][xOfBlks] != 0)
                return true;
    return false;
}

template <typename T>
inline bool checkIfNonZero(const T *seq, int count)
{
    while (count--)
        if (*seq++ != 0)
            return true;
            
    return false;
}

#endif
