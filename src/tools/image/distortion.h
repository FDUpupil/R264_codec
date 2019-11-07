#ifndef __DISTORTION_H__
#define __DISTORTION_H__

#include "common/type.h"
#include "common/math.h"

template <typename T>
CostType SAD4x4(const Blk<T, 4, 4> &res)
{
    CostType ret = 0;
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            ret += (CostType)Abs(res[y][x]);
    return ret;
}

template <typename T>
CostType SAD4x4(const Blk<T, 4, 4> &org, const Blk<T, 4, 4> &pred)
{
    CostType ret = 0;
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            ret += (CostType)Abs(org[y][x] - pred[y][x]);
    return ret;
}

template <typename T>
CostType SAD8x8(const Blk<T, 8, 8> &res)
{
    CostType ret = 0;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            ret += (CostType)Abs(res[y][x]);
    return ret;
}

template <typename T>
CostType SAD8x8(const Blk<T, 8, 8> &org, const Blk<T, 8, 8> &pred)
{
    CostType ret = 0;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            ret += (CostType)Abs(org[y][x] - pred[y][x]);
    return ret;
}

template <typename T>
CostType SSD4x4(const Blk<T, 4, 4> &res)
{
    CostType ret = 0;
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            ret += (CostType)Sqr<int32_t>(res[y][x]);
    return ret;
}

template <typename T>
CostType SSD4x4(const Blk<T, 4, 4> &org, const Blk<T, 4, 4> &pred)
{
    CostType ret = 0;
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            ret += (CostType)Sqr<int32_t>(org[y][x] - pred[y][x]);
    return ret;
}

template <typename T>
CostType SSD8x8(const Blk<T, 8, 8> &res)
{
    CostType ret = 0;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            ret += (CostType)Sqr<int32_t>(res[y][x]);
    return ret;
}

template <typename T>
CostType SSD8x8(const Blk<T, 8, 8> &org, const Blk<T, 8, 8> &pred)
{
    CostType ret = 0;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            ret += (CostType)Sqr<int32_t>(org[y][x] - pred[y][x]);
    return ret;
}

template <typename T>
CostType SATD4x4(const Blk<T, 4, 4> &res)
{
    Blk<CoefType, 4, 4> tmp;
    CoefType p[4];
    CostType ret = 0;

    for (int y = 0; y < 4; ++y) {
        p[0] = res[y][0] + res[y][3];
        p[1] = res[y][0] - res[y][3];
        p[2] = res[y][1] + res[y][2];
        p[3] = res[y][1] - res[y][2];

        tmp[y][0] = p[0] + p[2];
        tmp[y][1] = p[1] + p[3];
        tmp[y][2] = p[0] - p[2];
        tmp[y][3] = p[1] - p[3];
    }

    for (int x = 0; x < 4; ++x) {
        p[0] = tmp[0][x] + tmp[3][x];
        p[1] = tmp[0][x] - tmp[3][x];
        p[2] = tmp[1][x] + tmp[2][x];
        p[3] = tmp[1][x] - tmp[2][x];

        ret += (CostType)Abs(p[0] + p[2]);
        ret += (CostType)Abs(p[1] + p[3]);
        ret += (CostType)Abs(p[0] - p[2]);
        ret += (CostType)Abs(p[1] - p[3]);
    }

    // Normalization
    return (ret + 2) >> 2;
}

template <typename T>
CostType SATD4x4(const Blk<T, 4, 4> &org, const Blk<T, 4, 4> &pred)
{
    Blk<CoefType, 4, 4> tmp;
    CoefType p[4];
    CostType ret = 0;

    for (int y = 0; y < 4; ++y) {
        p[0] = (org[y][0] - pred[y][0]) + (org[y][3] - pred[y][3]);
        p[1] = (org[y][0] - pred[y][0]) - (org[y][3] - pred[y][3]);
        p[2] = (org[y][1] - pred[y][1]) + (org[y][2] - pred[y][2]);
        p[3] = (org[y][1] - pred[y][1]) - (org[y][2] - pred[y][2]);

        tmp[y][0] = p[0] + p[2];
        tmp[y][1] = p[1] + p[3];
        tmp[y][2] = p[0] - p[2];
        tmp[y][3] = p[1] - p[3];
    }

    for (int x = 0; x < 4; ++x) {
        p[0] = tmp[0][x] + tmp[3][x];
        p[1] = tmp[0][x] - tmp[3][x];
        p[2] = tmp[1][x] + tmp[2][x];
        p[3] = tmp[1][x] - tmp[2][x];

        ret += (CostType)Abs(p[0] + p[2]);
        ret += (CostType)Abs(p[1] + p[3]);
        ret += (CostType)Abs(p[0] - p[2]);
        ret += (CostType)Abs(p[1] - p[3]);
    }

    // Normalization
    return (ret + 2) >> 2;
}

template <typename T>
CostType SATD8x8(const Blk<T, 8, 8> &res)
{
    Blk<CoefType, 8, 8> tmp;
    CoefType p[8];
    CoefType q[8];
    CostType ret = 0;

    for (int y = 0; y < 8; ++y) {
        p[0] = res[y][0] + res[y][7];
        p[1] = res[y][0] - res[y][7];
        p[2] = res[y][3] + res[y][4];
        p[3] = res[y][3] - res[y][4];
        p[4] = res[y][1] + res[y][6];
        p[5] = res[y][1] - res[y][6];
        p[6] = res[y][2] + res[y][5];
        p[7] = res[y][2] - res[y][5];

        q[0] = p[0] + p[2];
        q[1] = p[1] + p[3];
        q[2] = p[0] - p[2];
        q[3] = p[1] - p[3];
        q[4] = p[4] + p[6];
        q[5] = p[5] + p[7];
        q[6] = p[4] - p[6];
        q[7] = p[5] - p[7];

        tmp[y][0] = q[0] + q[4];
        tmp[y][1] = q[1] + q[5];
        tmp[y][2] = q[2] + q[6];
        tmp[y][3] = q[3] + q[7];
        tmp[y][4] = q[0] - q[4];
        tmp[y][5] = q[1] - q[5];
        tmp[y][6] = q[2] - q[6];
        tmp[y][7] = q[3] - q[7];
    }

    for (int x = 0; x < 8; ++x) {
        p[0] = tmp[0][x] + tmp[7][x];
        p[1] = tmp[0][x] - tmp[7][x];
        p[2] = tmp[3][x] + tmp[4][x];
        p[3] = tmp[3][x] - tmp[4][x];
        p[4] = tmp[1][x] + tmp[6][x];
        p[5] = tmp[1][x] - tmp[6][x];
        p[6] = tmp[2][x] + tmp[5][x];
        p[7] = tmp[2][x] - tmp[5][x];

        q[0] = p[0] + p[2];
        q[1] = p[1] + p[3];
        q[2] = p[0] - p[2];
        q[3] = p[1] - p[3];
        q[4] = p[4] + p[6];
        q[5] = p[5] + p[7];
        q[6] = p[4] - p[6];
        q[7] = p[5] - p[7];

        ret += (CostType)Abs(q[0] + q[4]);
        ret += (CostType)Abs(q[1] + q[5]);
        ret += (CostType)Abs(q[2] + q[6]);
        ret += (CostType)Abs(q[3] + q[7]);
        ret += (CostType)Abs(q[0] - q[4]);
        ret += (CostType)Abs(q[1] - q[5]);
        ret += (CostType)Abs(q[2] - q[6]);
        ret += (CostType)Abs(q[3] - q[7]);
    }

    // Normalization
    return (ret + 4) >> 3;
}

template <typename T>
CostType SATD8x8(const Blk<T, 8, 8> &org, const Blk<T, 8, 8> &pred)
{
    Blk<CoefType, 8, 8> tmp;
    CoefType p[8];
    CoefType q[8];
    CostType ret = 0;

    for (int y = 0; y < 8; ++y) {
        p[0] = (org[y][0] - pred[y][0]) + (org[y][7] - pred[y][7]);
        p[1] = (org[y][0] - pred[y][0]) - (org[y][7] - pred[y][7]);
        p[2] = (org[y][3] - pred[y][3]) + (org[y][4] - pred[y][4]);
        p[3] = (org[y][3] - pred[y][3]) - (org[y][4] - pred[y][4]);
        p[4] = (org[y][1] - pred[y][1]) + (org[y][6] - pred[y][6]);
        p[5] = (org[y][1] - pred[y][1]) - (org[y][6] - pred[y][6]);
        p[6] = (org[y][2] - pred[y][2]) + (org[y][5] - pred[y][5]);
        p[7] = (org[y][2] - pred[y][2]) - (org[y][5] - pred[y][5]);

        q[0] = p[0] + p[2];
        q[1] = p[1] + p[3];
        q[2] = p[0] - p[2];
        q[3] = p[1] - p[3];
        q[4] = p[4] + p[6];
        q[5] = p[5] + p[7];
        q[6] = p[4] - p[6];
        q[7] = p[5] - p[7];

        tmp[y][0] = q[0] + q[4];
        tmp[y][1] = q[1] + q[5];
        tmp[y][2] = q[2] + q[6];
        tmp[y][3] = q[3] + q[7];
        tmp[y][4] = q[0] - q[4];
        tmp[y][5] = q[1] - q[5];
        tmp[y][6] = q[2] - q[6];
        tmp[y][7] = q[3] - q[7];
    }

    for (int x = 0; x < 8; ++x) {
        p[0] = tmp[0][x] + tmp[7][x];
        p[1] = tmp[0][x] - tmp[7][x];
        p[2] = tmp[3][x] + tmp[4][x];
        p[3] = tmp[3][x] - tmp[4][x];
        p[4] = tmp[1][x] + tmp[6][x];
        p[5] = tmp[1][x] - tmp[6][x];
        p[6] = tmp[2][x] + tmp[5][x];
        p[7] = tmp[2][x] - tmp[5][x];

        q[0] = p[0] + p[2];
        q[1] = p[1] + p[3];
        q[2] = p[0] - p[2];
        q[3] = p[1] - p[3];
        q[4] = p[4] + p[6];
        q[5] = p[5] + p[7];
        q[6] = p[4] - p[6];
        q[7] = p[5] - p[7];

        ret += (CostType)Abs(q[0] + q[4]);
        ret += (CostType)Abs(q[1] + q[5]);
        ret += (CostType)Abs(q[2] + q[6]);
        ret += (CostType)Abs(q[3] + q[7]);
        ret += (CostType)Abs(q[0] - q[4]);
        ret += (CostType)Abs(q[1] - q[5]);
        ret += (CostType)Abs(q[2] - q[6]);
        ret += (CostType)Abs(q[3] - q[7]);
    }

    // Normalization
    return (ret + 4) >> 3;
}

template <typename T, int width, int height>
DistType compute_SSE_macroblock(const Blk<T, width, height> &org, const Blk<T, width, height> &pred)
{   
    DistType dist_sse = 0;
    for(int y=0; y<height; y++){
        for(int x=0; x<width; x++)
            dist_sse += (DistType)Sqr<int32_t>(org[y][x] - pred[y][x]);
    }
    return dist_sse;
}

#endif
