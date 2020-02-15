#pragma once

#include "types.h"
#include <vector>

class Border
{
public:
    void graph(int t) { graph_ = t; }
    int graph() const  { return graph_; }

    void points(const std::vector<Point2d32i>& ps) { points_ = ps; }
    std::vector<Point2d32i> points() const { return points_; }
    std::vector<Point2d32i>& points() { return points_; }

private:
    int graph_ = 0;
    std::vector<Point2d32i> points_;
};
