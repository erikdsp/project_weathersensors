#ifndef WEATHER_SENSORS_STRUCTS_GLOBALS_H
#define WEATHER_SENSORS_STRUCTS_GLOBALS_H

#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <mutex>
#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std::literals::chrono_literals;



struct TimeDouble {
    std::chrono::system_clock::time_point time_point;
    double value;
};

// struct is used in SensorData class
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

// struct is used in SensorData class
struct SensorStatistics {
    Stats temperature;
    Stats humidity;
    Stats windspeed;
};




#endif