#ifndef WEATHER_SENSORS_THREADS_H
#define WEATHER_SENSORS_THREADS_H

#include "structs.h"
#include "SensorData.h"
#include "globals.h"
#include "DataGenerator.h"

/**
 *  Each function is used in a separate Thread
 *  Mutex Lockguard is used where necessary to avoid data races
 */

void sensor_temperature();
void sensor_humidity();
void sensor_windspeed();
void sensor_statistics();
void print_sensor_data();
void quit_prompt();

#endif