#include "globals.h"

std::mutex sensor_mutex;
std::atomic_bool system_running { true };  

/** 
 *  Global object to store data
 */
namespace sensor_data {
    SensorData sensor;
}
