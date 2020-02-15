#include "judge.h"
#include "base/log.h"
#include <cstdio>
#include <cmath>

using namespace std;

int pnpoly (int nvert, int* vertx, int* verty, int testx, int testy) 
{
    int i, j, c = 0;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ( (verty[i]>testy) != (verty[j]>testy) ) &&
             (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
    }
    return c;
}

bool judge_out_rectangle(Point& trail_points, const std::vector<Point>& original_points)
{
    //遍历原始警戒线的轨迹点，提取点到curves_points
    int rectangle_points_x[4], rectangle_points_y[4];
    rectangle_points_x[0] = original_points[0].x + GAP;
    rectangle_points_x[1] = original_points[0].x + GAP;
    rectangle_points_x[2] = original_points[1].x - GAP;
    rectangle_points_x[3] = original_points[1].x - GAP;
    rectangle_points_y[0] = original_points[0].y + GAP;
    rectangle_points_y[1] = original_points[1].y - GAP;
    rectangle_points_y[2] = original_points[1].y - GAP;
    rectangle_points_y[3] = original_points[0].y + GAP;
    return pnpoly(4, rectangle_points_x, rectangle_points_y, trail_points.x, trail_points.y) == 1;
}

bool judge_out_polygon(Point& trail_points, const std::vector<Point>& original_points)
{
    int length = original_points.size();
    std::vector<int> polygon_points_x(length);
    std::vector<int> polygon_points_y(length);
    for (int i = 0; i < length; i++)
    {
        polygon_points_x[i] = original_points[i].x;
        polygon_points_y[i] = original_points[i].y;
    }
    if(pnpoly(length, polygon_points_x, polygon_points_y, trail_points.x, trail_points.y) == 1)
    {
        return true;
        
    }
    else
    {
        return false;
    }
}


//=====================================================================
//判断两条线段是否相交，快速筛选以及跨立实验
bool cross_line(Point& a, Point& b, Point& c, Point& d){
    if (!(min(a.x, b.x) <= max(c.x, d.x) && min(c.y, d.y) <= max(a.y, b.y) &&
          min(c.x, d.x) <= max(a.x, b.x) && min(a.y, b.y) <= max(c.y, d.y)))
        return false;

    double u,v,w,z;//分别记录两个向量
    u=(c.x-a.x)*(b.y-a.y)-(b.x-a.x)*(c.y-a.y);
    v=(d.x-a.x)*(b.y-a.y)-(b.x-a.x)*(d.y-a.y);
    w=(a.x-c.x)*(d.y-c.y)-(d.x-c.x)*(a.y-c.y);
    return u*v<=1e-9 && w*z<=1e-9;
}


bool judge_out_broken_line(std::vector<Point>&& trail_points, std::vector<Point>&& broken_line_points){
    if(trail_points.size()<2){
        return false;
    }
    std::vector<Point>::iterator it, ot;
    //判断行人轨迹people_trail和误差警戒线是否产生交点，即越界curves_point
    if(trail_points.size() <= SEP){   // 若行人轨迹点数少于SEP, 则轨迹近视拟合成首尾点构成的线段
        for (it = broken_line_points.begin(); it != broken_line_points.end()-1; it += 1){
            if (cross_line(*trail_points.begin(), *(trail_points.end()-1), *it, *(it+1)))
                return true;
        }
    }else{
        //  计算时对行人轨迹的采样步长为SEP
        for (it = broken_line_points.begin(); it != broken_line_points.end()-1; it += 1){
            for (ot = trail_points.begin(); ot < trail_points.end()-SEP; ot += SEP){
                if (cross_line(*trail_points.begin(), *(ot+SEP), *it, *(it+1))){
                    return true;
                }
            }
            if(cross_line(*trail_points.begin(), *(trail_points.end()-1), *it, *(it+1))){
                return true;
            }
        }
    }
    return false;
}

bool judge_out_line(std::vector<Point>&& trail_points, std::vector<Point>&& line_points){
    if(trail_points.size()<2){
        return false;
    }
    std::vector<Point>::iterator it;
    //判断行人轨迹people_trail和误差警戒线是否产生交点，即越界
    if(trail_points.size() <= SEP){     // 若行人轨迹点数少于SEP, 则轨迹近视拟合成首尾点构成的线段
        if (cross_line(*trail_points.begin(), *(trail_points.end()-1), line_points[0], line_points[1]))
            return true;
    }else{
        //  计算时对行人轨迹的采样步长为SEP
        for (it = trail_points.begin(); it < trail_points.end()-SEP; it += SEP) {
            //判断是否有中越界
            if (cross_line(*trail_points.begin(), *(it + SEP), line_points[0], line_points[1])){
                return true;
            }
        }
        if (cross_line(*trail_points.begin(), *(trail_points.end()-1), line_points[0], line_points[1]))
            return true;
    }
    return false;
}
