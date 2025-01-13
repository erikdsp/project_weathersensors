#include "structs.h"
#include "SensorData.h"
#include "globals.h"
#include "DataGenerator.h"
#include "threads.h"
#include "SaveJson.h"
#include <iostream>

/**
 *  Global variables declared as extern to be available also in this file
 */
extern std::atomic_bool system_running;

namespace sensor_data {
    extern SensorData sensor;
}

int main() 
{
    std::thread temperature(sensor_temperature);
    std::thread relative_humidity(sensor_humidity);
    std::thread windspeed(sensor_windspeed);
    std::thread statistics(sensor_statistics);
    std::thread print_data(print_sensor_data);
    std::thread user_prompt(quit_prompt);

    user_prompt.join();
    temperature.join();
    relative_humidity.join();
    windspeed.join();
    system_running = false;
    statistics.join();
    print_data.join();

    std::cout << "STOPPING SENSOR MONITORING\n";
    std::string filename = save_sensordata_to_json("SensorData", sensor_data::sensor);
    std::cout << "Data saved to " << filename << "\n";


    return 0;
}
