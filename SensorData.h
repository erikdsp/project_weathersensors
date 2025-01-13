#ifndef WEATHER_SENSORS_SENSORDATA_H
#define WEATHER_SENSORS_SENSORDATA_H
#include "structs.h"
#include "globals.h"
#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

/**
 *  Class to store and manipulate sensor data
 *  Specifically: Temperature, Humidity, Wind Speed
 *  New sensor data is stored in m_new_readings
 *  calculate_statistics() updates m_statistics with data from m_new_readings
 *  move_sensor_data() moves data from m_new_readings to m_readings
 *  std::lock_guard<std::mutex> used where needed
 */

class SensorData {
private:
    SensorReadings m_new_readings;
    SensorReadings m_readings;
    SensorStatistics m_statistics;

    void calculate_statistics(Stats& stat, bool& first_reading, 
                              const std::vector<TimeDouble>& readings, 
                              const std::vector<TimeDouble>& new_reading);
    void move_sensor_data(std::vector<TimeDouble>& readings, std::vector<TimeDouble>& new_readings);
    void print_reading(const std::vector<TimeDouble>& readings, const std::vector<TimeDouble>& new_readings);
    void print_single_statistic(Stats stat);
    void store_new_reading(double reading, std::vector<TimeDouble>& readings);
    std::string timepoint_to_string(std::chrono::system_clock::time_point time_point) const;
public:
    void store_temperature_reading(double reading);
    void store_humidity_reading(double reading);
    void store_windspeed_reading(double reading);
    void calculate_temperature_statistic(bool& first_reading);
    void calculate_humidity_statistic(bool& first_reading);
    void calculate_windspeed_statistic(bool& first_reading);
    void move_temperature_data();
    void move_humidity_data();
    void move_windspeed_data();    
    void print_latest_readings();
    void print_statistics();
    json construct_json_object() const;
};


#endif