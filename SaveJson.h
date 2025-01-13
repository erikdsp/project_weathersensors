#ifndef WEATHER_SENSORS_SAVEJSON_H
#define WEATHER_SENSORS_SAVEJSON_H
#include "nlohmann/json.hpp"
#include "globals.h"
#include "SensorData.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
using json = nlohmann::ordered_json;



bool file_exists(const std::string& filename); 
std::string get_current_date_cstyle();
std::string generate_free_json_filename(const std::string& filename);
std::string save_sensordata_to_json(const std::string& filename, const SensorData& data);

#endif