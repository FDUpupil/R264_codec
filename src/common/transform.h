#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "type.h"

template <typename T1, typename T2>
void IDCT4x4(const Blk<T1, 4, 4> &src, Blk<T2, 4, 4> &dst)
{
    Blk<T1, 4, 4> tmp;
    T1 p[4];

    for (int y = 0; y < 4; ++y) {
        p[0] = src[y][0] + src[y][2];
        p[1] = src[y][0] - src[y][2];
        p[2] = (src[y][1] >> 1) - src[y][3];
        p[3] = src[y][1] + (src[y][3] >> 1);

        tmp[y][0] = p[0] + p[3];
        tmp[y][1] = p[1] + p[2];
        tmp[y][2] = p[1] - p[2];
        tmp[y][3] = p[0] - p[3];
    }

    for (int x = 0; x < 4; ++x) {
        p[0] = tmp[0][x] + tmp[2][x];
        p[1] = tmp[0][x] - tmp[2][x];
        p[2] = (tmp[1][x] >> 1) - tmp[3][x];
        p[3] = tmp[1][x] + (tmp[3][x] >> 1);

        dst[0][x] = T2((p[0] + p[3] + (1 << 5)) >> 6);
        dst[1][x] = T2((p[1] + p[2] + (1 << 5)) >> 6);
        dst[2][x] = T2((p[1] - p[2] + (1 << 5)) >> 6);
        dst[3][x] = T2((p[0] - p[3] + (1 << 5)) >> 6);
    }
}

template <typename T1, typename T2>
void IDCT8x8(const Blk<T1, 8, 8> &src, Blk<T2, 8, 8> &dst)
{
    Blk<T1, 8, 8> tmp;
    T1 p[8];
    T1 q[8];

    for (int y = 0; y < 8; ++y) {
        p[0] = src[y][0] + src[y][4];
        p[1] = -src[y][3] + src[y][5] - src[y][7] - (src[y][7] >> 1);
        p[2] = src[y][0] - src[y][4];
        p[3] = src[y][1] + src[y][7] - src[y][3] - (src[y][3] >> 1);
        p[4] = (src[y][2] >> 1) - src[y][6];
        p[5] = -src[y][1] + src[y][7] + src[y][5] + (src[y][5] >> 1);
        p[6] = src[y][2] + (src[y][6] >> 1);
        p[7] = src[y][3] + src[y][5] + src[y][1] + (src[y][1] >> 1);

        q[0] = p[0] + p[6];
        q[1] = p[1] + (p[7] >> 2);
        q[2] = p[2] + p[4];
        q[3] = p[3] + (p[5] >> 2);
        q[4] = p[2] - p[4];
        q[5] = (p[3] >> 2) - p[5];
        q[6] = p[0] - p[6];
        q[7] = p[7] - (p[1] >> 2);

        tmp[y][0] = q[0] + q[7];
        tmp[y][1] = q[2] + q[5];
        tmp[y][2] = q[4] + q[3];
        tmp[y][3] = q[6] + q[1];
        tmp[y][4] = q[6] - q[1];
        tmp[y][5] = q[4] - q[3];
        tmp[y][6] = q[2] - q[5];
        tmp[y][7] = q[0] - q[7];
    }

    for (int x = 0; x < 8; ++x) {
        p[0] = tmp[0][x] + tmp[4][x];
        p[1] = -tmp[3][x] + tmp[5][x] - tmp[7][x] - (tmp[7][x] >> 1);
        p[2] = tmp[0][x] - tmp[4][x];
        p[3] = tmp[1][x] + tmp[7][x] - tmp[3][x] - (tmp[3][x] >> 1);
        p[4] = (tmp[2][x] >> 1) - tmp[6][x];
        p[5] = -tmp[1][x] + tmp[7][x] + tmp[5][x] + (tmp[5][x] >> 1);
        p[6] = tmp[2][x] + (tmp[6][x] >> 1);
        p[7] = tmp[3][x] + tmp[5][x] + tmp[1][x] + (tmp[1][x] >> 1);

        q[0] = p[0] + p[6];
        q[1] = p[1] + (p[7] >> 2);
        q[2] = p[2] + p[4];
        q[3] = p[3] + (p[5] >> 2);
        q[4] = p[2] - p[4];
        q[5] = (p[3] >> 2) - p[5];
        q[6] = p[0] - p[6];
        q[7] = p[7] - (p[1] >> 2);

        dst[0][x] = T2((q[0] + q[7] + 32) >> 6);
        dst[1][x] = T2((q[2] + q[5] + 32) >> 6);
        dst[2][x] = T2((q[4] + q[3] + 32) >> 6);
        dst[3][x] = T2((q[6] + q[1] + 32) >> 6);
        dst[4][x] = T2((q[6] - q[1] + 32) >> 6);
        dst[5][x] = T2((q[4] - q[3] + 32) >> 6);
        dst[6][x] = T2((q[2] - q[5] + 32) >> 6);
        dst[7][x] = T2((q[0] - q[7] + 32) >> 6);
    }
}

template <typename T1, typename T2>
void IWHT2x2(const Blk<T1, 2, 2> &src, Blk<T2, 2, 2> &dst)
{
    Blk<T1, 2, 2> tmp;

    tmp[0][0] = src[0][0] + src[0][1];
    tmp[0][1] = src[0][0] - src[0][1];
    tmp[1][0] = src[1][0] + src[1][1];
    tmp[1][1] = src[1][0] - src[1][1];

    dst[0][0] = T2(tmp[0][0] + tmp[1][0]);
    dst[1][0] = T2(tmp[0][0] - tmp[1][0]);
    dst[0][1] = T2(tmp[0][1] + tmp[1][1]);
    dst[1][1] = T2(tmp[0][1] - tmp[1][1]);
}

template <typename T1, typename T2>
void IWHT2x4(const Blk<T1, 2, 4> &src, Blk<T2, 2, 4> &dst)
{
    Blk<T2, 2, 4> tmp;
    T2 p[4];

    for (int y = 0; y < 4; ++y) {
        tmp[y][0] = src[y][0] + src[y][1];
        tmp[y][1] = src[y][0] - src[y][1];
    }

    for (int x = 0; x < 2; ++x) {
        p[0] = tmp[0][x] + tmp[2][x];
        p[1] = tmp[1][x] + tmp[3][x];
        p[2] = tmp[0][x] - tmp[2][x];
        p[3] = tmp[1][x] - tmp[3][x];

        dst[0][x] = p[0] + p[1];
        dst[1][x] = p[2] + p[3];
        dst[2][x] = p[2] - p[3];
        dst[3][x] = p[0] - p[1];
    }
}

template <typename T1, typename T2>
void IWHT4x4(const Blk<T1, 4, 4> &src, Blk<T2, 4, 4> &dst)
{
    Blk<T2, 4, 4> tmp;
    T2 p[4];

    for (int y = 0; y < 4; ++y) {
        p[0] = src[y][0] + src[y][2];
        p[1] = src[y][1] + src[y][3];
        p[2] = src[y][0] - src[y][2];
        p[3] = src[y][1] - src[y][3];

        tmp[y][0] = p[0] + p[1];
        tmp[y][1] = p[2] + p[3];
        tmp[y][2] = p[2] - p[3];
        tmp[y][3] = p[0] - p[1];
    }

    for (int x = 0; x < 4; ++x) {
        p[0] = tmp[0][x] + tmp[2][x];
        p[1] = tmp[1][x] + tmp[3][x];
        p[2] = tmp[0][x] - tmp[2][x];
        p[3] = tmp[1][x] - tmp[3][x];

        dst[0][x] = p[0] + p[1];
        dst[1][x] = p[2] + p[3];
        dst[2][x] = p[2] - p[3];
        dst[3][x] = p[0] - p[1];
    }
}

#endif
