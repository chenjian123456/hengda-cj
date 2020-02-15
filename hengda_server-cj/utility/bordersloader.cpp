#include "bordersloader.h"
#include "base/log.h"
#include <fstream>
#include <sstream>

void BordersLoader::save(const json11::Json& borders,const std::string filename)
{
    borders_ = borders;

    std::ofstream file(filename, std::ofstream::out);
    if(file) {
        file << borders.dump();
    }
    file.close();
}

std::map<std::string, std::vector<Border>> BordersLoader::load()
{
    std::map<std::string, std::vector<Border>> stacks;

    std::string text;
    std::ifstream file("border.json", std::ifstream::in);
    if(file) {
        std::stringstream stream;
        stream << file.rdbuf();
        text = stream.str();
    }
    file.close();

    LOG << "borders: " << text;

    std::string err;
    borders_ = json11::Json::parse(text, err);
    if(!err.empty()) {
        borders_ = json11::Json::array();
        ELOG << "Parse borders.json failed!";
    }

    for(const auto& stack_json : borders_.array_items()) {
        auto key = stack_json["name"].string_value();
        stacks[key] = std::vector<Border>();

        for(const auto& border_json: stack_json["value"].array_items()) {
            Border border;
            border.graph(border_json["type"].int_value());

            for(const auto& point_json : border_json["points"].array_items()) {
                Point2d32i point;
                point.x = point_json[0].int_value();
                point.y = point_json[1].int_value();
                border.points().push_back(point);
                LOG << "<" << point.x << ", " << point.y << ">";
            }
            stacks[key].push_back(border);
        }
    }

    return std::move(stacks);
}


std::vector<Border> BordersLoader::load(std::string filename)
{
    std::vector<Border> border;

    std::string text;
    std::ifstream file(filename, std::ifstream::in);
    if(file) {
        std::stringstream stream;
        stream << file.rdbuf();
        text = stream.str();
    }
    file.close();

    LOG << "borders: " << text;

    std::string err;
    borders_ = json11::Json::parse(text, err);
    if(!err.empty()) {
        borders_ = json11::Json::array();
        ELOG << "Parse borders.json failed!";
    }


    for(const auto& stack_json : borders_.array_items()){
        Border border_;
        border_.graph(stack_json["type"].int_value());

        for(const auto& point_json : stack_json["points"].array_items()){
            Point2d32i point;
            point.x = point_json[0].int_value();
            point.y = point_json[1].int_value();
            border_.points().push_back(point);
            LOG << "<" << point.x << ", " << point.y << ">";
        }

        border.push_back(border_); 
    }



    return std::move(border);
}
