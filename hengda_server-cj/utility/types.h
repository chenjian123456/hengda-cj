#pragma once

#include <stdint.h>

template<typename Type>
struct _Point
{
    Type x;
    Type y;
};

using Point  = _Point<int>;
using PointF = _Point<float>;

using Point2d32i = _Point<int32_t>;
using Point2d64i = _Point<int64_t>;
using Point2d32f = _Point<float>;
using Point2d64f = _Point<double>;

enum Graph {
    NONE,
    RECTANGLE       = 1,
    ELLIPSE         = 2,
    LINE            = 4,
    CURVES          = 8,
    POLYGON         = 16,
    BROKEN_LINE     = 32
};
