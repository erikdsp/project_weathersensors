#ifndef WEATHER_SENSORS_STRUCTS_GLOBALS_H
#define WEATHER_SENSORS_STRUCTS_GLOBALS_H

// #include "SensorData.h"
// #include "threads.h"
// #include "DataGenerator.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>
#include <chrono>
#include <vector>
#include <map>
#include <algorithm>
using namespace std::literals::chrono_literals;



struct TimeDouble {
    std::chrono::system_clock::time_point time_point;
    double value;
};

struct SensorReadings {
    std::vector<TimeDouble> temperature;
    std::vector<TimeDouble> humidity;
    std::vector<TimeDouble> windspeed;
};

struct Stats {
    TimeDouble max;
    TimeDouble min;
    double average;
};

struct SensorStatistics {
    Stats temperature;
    Stats humidity;
    Stats windspeed;
};




#endif