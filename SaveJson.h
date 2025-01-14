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


// checks if file exists in current directory
bool file_exists(const std::string& filename); 

// get current date in format suitable for a filename
std::string get_current_date_cstyle();

// Function takes a base filename, adds the current date and .json,
// if file already exists it adds integers starting with 1
// until it finds an unused filename
std::string generate_free_json_filename(const std::string& filename);

// function use SensorData methods to construct a json object,
// generates a filenamne and saves it in the current folder
std::string save_sensordata_to_json(const std::string& filename, const SensorData& data);

#endif