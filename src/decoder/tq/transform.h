#ifndef __ENCODER_TRANSFORM_H__
#define __ENCODER_TRANSFORM_H__

#include "common/transform.h"

template <typename T1, typename T2>
void DCT4x4(const Blk<T1, 4, 4> &src, Blk<T2, 4, 4> &dst)
{
    Blk<T2, 4, 4> tmp;
    T2 p[4];

    for (int y = 0; y < 4; ++y) {
        p[0] = src[y][0] + src[y][3];
        p[1] = src[y][1] + src[y][2];
        p[2] = src[y][1] - src[y][2];
        p[3] = src[y][0] - src[y][3];

        tmp[y][0] = p[0] + p[1];
        tmp[y][1] = p[2] + (p[3] << 1);
        tmp[y][2] = p[0] - p[1];
        tmp[y][3] = -(p[2] << 1) + p[3];
    }

    for (int x = 0; x < 4; ++x) {
        p[0] = tmp[0][x] + tmp[3][x];
        p[1] = tmp[1][x] + tmp[2][x];
        p[2] = tmp[1][x] - tmp[2][x];
        p[3] = tmp[0][x] - tmp[3][x];

        dst[0][x] = p[0] + p[1];
        dst[1][x] = p[2] + (p[3] << 1);
        dst[2][x] = p[0] - p[1];
        dst[3][x] = -(p[2] << 1) + p[3];
    }
}

template <typename T1, typename T2>
void DCT8x8(const Blk<T1, 8, 8> &src, Blk<T2, 8, 8> &dst)
{
    Blk<T2, 8, 8> tmp;
    T2 p[8];
    T2 q[8];

    for (int y = 0; y < 8; ++y) {
        p[0] = src[y][0] + src[y][7];
        p[1] = src[y][3] - src[y][4];
        p[2] = src[y][1] + src[y][6];
        p[3] = src[y][2] - src[y][5];
        p[4] = src[y][2] + src[y][5];
        p[5] = src[y][1] - src[y][6];
        p[6] = src[y][3] + src[y][4];
        p[7] = src[y][0] - src[y][7];

        q[0] = p[0] + p[6];
        q[1] = p[1] - (p[7] >> 2);
        q[2] = p[2] + p[4];
        q[3] = p[3] + (p[5] >> 2);
        q[4] = p[2] - p[4];
        q[5] = (p[3] >> 2) - p[5];
        q[6] = p[0] - p[6];
        q[7] = (p[1] >> 2) + p[7];

        tmp[y][0] = q[0] + q[2];
        tmp[y][1] = q[3] - q[5] + q[7] + (q[7] >> 1);
        tmp[y][2] = (q[4] >> 1) + q[6];
        tmp[y][3] = -q[1] - q[3] - (q[3] >> 1) + q[7];
        tmp[y][4] = q[0] - q[2];
        tmp[y][5] = q[1] + q[5] + (q[5] >> 1) + q[7];
        tmp[y][6] = -q[4] + (q[6] >> 1);
        tmp[y][7] = -q[1] - (q[1] >> 1) + q[3] + q[5];
    }

    for (int x = 0; x < 8; ++x) {
        p[0] = tmp[0][x] + tmp[7][x];
        p[1] = tmp[3][x] - tmp[4][x];
        p[2] = tmp[1][x] + tmp[6][x];
        p[3] = tmp[2][x] - tmp[5][x];
        p[4] = tmp[2][x] + tmp[5][x];
        p[5] = tmp[1][x] - tmp[6][x];
        p[6] = tmp[3][x] + tmp[4][x];
        p[7] = tmp[0][x] - tmp[7][x];

        q[0] = p[0] + p[6];
        q[1] = p[1] - (p[7] >> 2);
        q[2] = p[2] + p[4];
        q[3] = p[3] + (p[5] >> 2);
        q[4] = p[2] - p[4];
        q[5] = (p[3] >> 2) - p[5];
        q[6] = p[0] - p[6];
        q[7] = (p[1] >> 2) + p[7];

        dst[0][x] = q[0] + q[2];
        dst[1][x] = q[3] - q[5] + q[7] + (q[7] >> 1);
        dst[2][x] = (q[4] >> 1) + q[6];
        dst[3][x] = -q[1] - q[3] - (q[3] >> 1) + q[7];
        dst[4][x] = q[0] - q[2];
        dst[5][x] = q[1] + q[5] + (q[5] >> 1) + q[7];
        dst[6][x] = -q[4] + (q[6] >> 1);
        dst[7][x] = -q[1] - (q[1] >> 1) + q[3] + q[5];
    }
}

template <typename T1, typename T2>
void WHT2x2(const Blk<T1, 2, 2> &src, Blk<T2, 2, 2> &dst)
{
    Blk<T2, 2, 2> tmp;

    tmp[0][0] = src[0][0] + src[0][1];
    tmp[0][1] = src[0][0] - src[0][1];
    tmp[1][0] = src[1][0] + src[1][1];
    tmp[1][1] = src[1][0] - src[1][1];

    dst[0][0] = tmp[0][0] + tmp[1][0];
    dst[1][0] = tmp[0][0] - tmp[1][0];
    dst[0][1] = tmp[0][1] + tmp[1][1];
    dst[1][1] = tmp[0][1] - tmp[1][1];
}

template <typename T1, typename T2>
void WHT2x4(const Blk<T1, 2, 4> &src, Blk<T2, 2, 4> &dst)
{
    Blk<T1, 2, 4> tmp;
    T1 p[4];

    for (int y = 0; y < 4; ++y) {
        tmp[y][0] = src[y][0] + src[y][1];
        tmp[y][1] = src[y][0] - src[y][1];
    }

    for (int x = 0; x < 2; ++x) {
        p[0] = tmp[0][x] + tmp[3][x];
        p[1] = tmp[0][x] - tmp[3][x];
        p[2] = tmp[1][x] + tmp[2][x];
        p[3] = tmp[1][x] - tmp[2][x];

        dst[0][x] = T2(p[0] + p[2]);
        dst[1][x] = T2(p[1] + p[3]);
        dst[2][x] = T2(p[0] - p[2]);
        dst[3][x] = T2(p[1] - p[3]);
    }
}

template <typename T1, typename T2>
void WHT4x4(const Blk<T1, 4, 4> &src, Blk<T2, 4, 4> &dst)
{
    Blk<T1, 4, 4> tmp;
    T1 p[4];

    for (int y = 0; y < 4; ++y) {
        p[0] = src[y][0] + src[y][3];
        p[1] = src[y][0] - src[y][3];
        p[2] = src[y][1] + src[y][2];
        p[3] = src[y][1] - src[y][2];

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

        // x264 use round_shift, while JM use only shift
        dst[0][x] = T2(p[0] + p[2]) >> 1;
        dst[1][x] = T2(p[1] + p[3]) >> 1;
        dst[2][x] = T2(p[0] - p[2]) >> 1;
        dst[3][x] = T2(p[1] - p[3]) >> 1;
    }
}

#endif
