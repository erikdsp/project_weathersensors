#ifndef WEATHER_SENSORS_DATAGENERATOR_H
#define WEATHER_SENSORS_DATAGENERATOR_H
#include "structs.h"
#include "SensorData.h"
#include "threads.h"
#include <random>

/**
 *  Class to generate fluctuating random numbers in a range
 *  Initializes a Mersienne Twister random engine (mt19937)
 *  get_initial_value() generates a number in the range
 *  get_new_value() generates a number close to the last one that does not exceed m_min and m_max
 */
class DataGenerator {
private:
    const double m_min;
    const double m_max;
    std::mt19937 m_random;
    std::uniform_real_distribution<> m_distrib;
    std::uniform_real_distribution<> m_fluct_distrib;
    double m_last_value{};
public:
    DataGenerator(double min = 0, double max = 100, double fluct_min = -0.1, double fluct_max = 0.1) 
        : m_min { min }, m_max{ max }, m_random { std::random_device{}() }, 
          m_distrib { m_min, m_max }, m_fluct_distrib { fluct_min, fluct_max } {}
    double get_initial_value();
    double get_new_value();
};


#endif