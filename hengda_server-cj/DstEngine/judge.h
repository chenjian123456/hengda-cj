#pragma once

#include "../utility/types.h"
#include <vector>

#define SEP (6)
#define GAP  (20)

bool cross_line(Point& a, Point& b, Point& c, Point& d);
bool judge_out_broken_line(std::vector<Point>&& trail_points, std::vector<Point>&& broken_line_points);
bool judge_out_line(std::vector<Point>&& trail_points, std::vector<Point>&& line_points);

// 判断人是否在多边形内部
int pnpoly (int nvert, int* vertx, int* verty, int testx, int testy);
bool judge_out_polygon(Point& trail_points, const std::vector<Point>& original_points);
bool judge_out_rectangle(Point& trail_points, const std::vector<Point>& original_points);
