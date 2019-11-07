#ifndef __MATH_H__
#define __MATH_H__

#include <cmath>

#include"type.h"

template <typename T>
inline T Abs(T x) { return x >= 0 ? x : -x; }

template <typename T>
inline T Ceil(T x) { return (T)std::ceil(x); }

template <typename T>
inline T Clip3(T x, T y, T z) { return (z < x) ? x : (z > y) ? y : z; }

template <typename T>
inline PixType Clip1(T x, int bitDepth) { return (PixType)Clip3<T>(T(0), T((1 << bitDepth) - 1), x); }

template <typename T>
inline T Floor(T x) { return (T)std::floor(x); }

template <typename T>
inline T Min(T x, T y) { return x <= y ? x : y; }

template <typename T>
inline T Max(T x, T y) { return x >= y ? x : y; }

template <typename T>
inline T Median(T x, T y, T z) { return x + y + z - Min(x, Min(y, z)) - Max(x, Max(y, z)); }

template <typename T>
inline T Round(T x) { return (T)std::round(x); }

template <typename T>
inline int8_t Sign(T x) { return x >= 0 ? 1 : -1; }

template <typename T1, typename T2>
inline T1 SignAbs(T1 x, T2 y) { return y >= 0 ? x : -x; }

template <typename T>
inline T Sqr(T x) { return x * x; }

template <typename T>
inline double Sqrt(T x) { return std::sqrt(x); }

template <typename T>
inline T RoundShift(T x, uint8_t rshift) { return (x + (1 << (rshift - 1))) >> rshift; }

//PSNR
inline  PSNRType psnr(PixType max_sample_sq, uint64_t samples, DistType sse_distortion) {return (PSNRType) (10.0 * log10(max_sample_sq * (double) ((double) samples / (sse_distortion < 1.0 ? 1.0 : sse_distortion))));}

#endif
