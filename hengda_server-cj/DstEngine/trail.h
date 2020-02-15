#pragma once

#define TIME_STAMP 120
#define MUCH 60
#define THRESOD 0.2
#define NORMALIZE 2203.0
#define THETA 0.5

#include "../utility/types.h"
#include <vector>
#include <math.h>

using std::min;
using std::max;

class Trail
{
public:
    Trail(Point x, Point y);
    bool check_states();
    void insert(Point a, Point b);
    void close_insertable();
    void open_insertable();
    bool get_insertable();
    std::vector<Point> points();
    float cal_dis(Point x, Point y);
    void update_states_insert(Point x, Point y);
    void update_states_common();
    void clear_();
    uint32_t count;
    bool insertable_;
    Point lt, rb;

private:
    std::vector<Point> points_;
};

Trail::Trail(Point a, Point b){
    lt = a;
    rb = b;
    count = 0;
    insertable_ = false;
    Point temp;
    temp.x = round(a.x + b.x) / 2;
    temp.y = b.y;
    points_.push_back(temp);
}

bool Trail::check_states(){
    if(count<TIME_STAMP){
        return true;
    }else return false;
}

void Trail::clear_(){ 
    points_.clear();
    Point temp;
    temp.x = round(lt.x + rb.x) / 2;
    temp.y = rb.y;
    points_.push_back(temp);
}

void Trail::close_insertable(){ insertable_ = false; }

bool Trail::get_insertable(){ return insertable_; }

void Trail::open_insertable(){ insertable_ = true; }

std::vector<Point> Trail::points(){ return points_; }

void Trail::insert(Point a, Point b){ 
    Point temp;
    temp.x = round(a.x + b.x) / 2;
    temp.y = b.y;
    points_.push_back(temp);
    update_states_insert(a, b);
}

void Trail::update_states_insert(Point x, Point y){
    count = 0;
    lt = x, rb = y;
    insertable_ = false;
    if(points_.size() > MUCH){
        points_.clear();
        Point temp;
        temp.x = round(lt.x + rb.x) / 2;
        temp.y = rb.y;
        points_.push_back(temp);
    }
}

void Trail::update_states_common(){
    count++;
    insertable_ = true;
}

float Trail::cal_dis(Point a, Point b){
    //iou 计算
    float w, h, SA, SB, iou, dis;
    w = min(b.x, rb.x) - max(a.x, lt.x);
    h = min(b.y, rb.y) - max(a.y, lt.y);
    if(w <= 0 || h <= 0)
        iou = 0.0;
    else{
        SA = (b.x - a.x) * (b.y - a.y);
        SB = (rb.x - lt.x) * (rb.y - lt.y);
        iou =  w*h/(SA + SB - w*h);
    }
    //欧氏距离计算
    float mid_1_x, mid_1_y, mid_2_x, mid_2_y;
    mid_1_x = (a.x+b.x)/2.0;
    mid_1_y = (a.y+b.y)/2.0;
    mid_2_x = (lt.x+rb.x)/2.0;
    mid_2_y = (lt.y+rb.y)/2.0;
    dis = sqrt(pow((mid_1_x - mid_2_x), 2) + pow((mid_1_y - mid_2_y), 2)) / NORMALIZE;
    
    //返回iou和欧式距离的加权值
    return THETA*iou - (1-THETA)*dis;
}
