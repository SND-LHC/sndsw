// This is the C++ version of shipLHC/rawData/boardMappingParser.py
#ifndef BOARDMAP_H
#define BOARDMAP_H 1

#include <iostream>
#include <string>
#include <map>
#include <tuple>
#include "nlohmann/json.hpp"

using std::map;
using std::string;
using json = nlohmann::json;

std::tuple<map<string, map<string, map<string, int>> >,
           map<string, map<string, map<string, string>> > > getBoardMapping(json j);

#endif
