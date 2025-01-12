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

std::mutex sensor_mutex;
std::atomic_bool system_running { true };  


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
};

/** 
 *  Global object to store data
 */
namespace sensor_data {
    SensorData sensor;
}

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

double DataGenerator::get_initial_value(){
    m_last_value = m_distrib(m_random);
    return m_last_value;
}

double DataGenerator::get_new_value(){

    double fluct { m_fluct_distrib(m_random) };
    double new_value = m_last_value += fluct;
    // check that value does not exceed bounds
    if ( new_value < m_min || new_value > m_max ) {
        new_value = m_last_value -= fluct;
    }
    return new_value;
}


void sensor_temperature()
{
    DataGenerator temperature_generator ( -15, 30, -0.2, 0.2 );
    double temperature { temperature_generator.get_initial_value() };

    for (int i = 0 ; i < 50 ; ++i) {
        temperature = temperature_generator.get_new_value();
        sensor_data::sensor.store_temperature_reading(temperature);
        std::this_thread::sleep_for(500ms);
    }

}

void sensor_humidity()
{
    DataGenerator humidity_generator ( 55.0, 100.0, -0.1, 0.1 );
    double humidity { humidity_generator.get_initial_value() };

    for (int i = 0 ; i < 50 ; ++i) {
        humidity = humidity_generator.get_new_value();
        sensor_data::sensor.store_humidity_reading(humidity);
        std::this_thread::sleep_for(500ms);
    }
}

void sensor_windspeed()
{
    DataGenerator windspeed_generator ( 0.0, 25.0, -0.5, 0.5 );
    double windspeed { windspeed_generator.get_initial_value() };

    for (int i = 0 ; i < 50 ; ++i) {
        windspeed = windspeed_generator.get_new_value();
        sensor_data::sensor.store_windspeed_reading(windspeed);
        std::this_thread::sleep_for(500ms);
    }    
}


void SensorData::store_new_reading(double reading, std::vector<TimeDouble>& readings) {
    std::lock_guard<std::mutex> guard(sensor_mutex);
    readings.emplace_back(std::chrono::system_clock::now(), reading);
}

void SensorData::store_temperature_reading(double reading){
    store_new_reading(reading, m_new_readings.temperature);
}

void SensorData::store_humidity_reading(double reading){
    store_new_reading(reading, m_new_readings.humidity);
}
void SensorData::store_windspeed_reading(double reading){
    store_new_reading(reading, m_new_readings.windspeed);
}


/**
 *  Calculates Max, Min and Average
 *  @param first_reading    If true set max and min, then update first_reading to false
 *  @param sum              Calculate sum of readings and new_readings to get average
 *  @param stat             Stats variable gets updated by the function
 */
void SensorData::calculate_statistics(Stats& stat, bool& first_reading,
    // Stats& stat, bool& first_reading,
    const std::vector<TimeDouble>& readings,
    const std::vector<TimeDouble>& new_readings) {

    double sum{ stat.average * readings.size() };
    for (auto& reading : new_readings) {
        if (first_reading) {
            // set max
            stat.max.value = reading.value;     
            stat.max.time_point = reading.time_point; 
            // set min
            stat.min.value = reading.value;
            stat.min.time_point = reading.time_point;
            first_reading = false;
        }
        // find and update max
        if (reading.value > stat.max.value) {
            stat.max.value = reading.value;     
            stat.max.time_point = reading.time_point; 
        }
        // find and update min
        if (reading.value < stat.min.value) {
            stat.min.value = reading.value;
            stat.min.time_point = reading.time_point;
        }
        // add value to sum
        sum += reading.value;
    }
    int num_of_entries { static_cast<int>(readings.size() + new_readings.size()) };
    stat.average = sum / num_of_entries;
}


void SensorData::calculate_temperature_statistic(bool& first_reading){
    calculate_statistics(m_statistics.temperature, first_reading, m_readings.temperature, m_new_readings.temperature);
}

void SensorData::calculate_humidity_statistic(bool& first_reading){
    calculate_statistics(m_statistics.humidity, first_reading, m_readings.humidity, m_new_readings.humidity);
}

void SensorData::calculate_windspeed_statistic(bool& first_reading){
    calculate_statistics(m_statistics.windspeed, first_reading, m_readings.windspeed, m_new_readings.windspeed);
}



/**
 *  Move from new_readings to readings 
 */
void SensorData::move_sensor_data( std::vector<TimeDouble>& readings, std::vector<TimeDouble>& new_readings ) {
    // move data into readings
    readings.insert(readings.end(), 
                    std::make_move_iterator(new_readings.begin()), 
                    std::make_move_iterator(new_readings.end()) );
    // clear new_readings
    new_readings.clear();
}

void SensorData::move_temperature_data(){
    move_sensor_data(m_readings.temperature, m_new_readings.temperature);
}
void SensorData::move_humidity_data(){
    move_sensor_data(m_readings.humidity, m_new_readings.humidity);
}
void SensorData::move_windspeed_data(){
    move_sensor_data(m_readings.windspeed, m_new_readings.windspeed);
}


void sensor_statistics() {

    static bool first_temperature { true };
    static bool first_humidity { true };
    static bool first_windspeed { true };
    
    while (system_running) {
        // sleep for 5000ms (== 5s)
        for (int i = 0 ; i < 50 && system_running ; i++){
            std::this_thread::sleep_for(100ms);
        }
        {
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::sensor.calculate_temperature_statistic(first_temperature);
            sensor_data::sensor.calculate_humidity_statistic(first_humidity);
            sensor_data::sensor.calculate_windspeed_statistic(first_windspeed);

            sensor_data::sensor.move_temperature_data();
            sensor_data::sensor.move_humidity_data();
            sensor_data::sensor.move_windspeed_data();
        }
    }
}


void SensorData::print_reading( const std::vector<TimeDouble>& readings, const std::vector<TimeDouble>& new_readings) {
    if (new_readings.size() > 0) {
        std::cout << new_readings.back().value << ", "
                  << new_readings.back().time_point;
    } else if (readings.size() > 0) {
        std::cout << readings.back().value << ", "
                  << readings.back().time_point;
    } else {
        std::cout << "<no sensor data>";
    }
}

void SensorData::print_latest_readings(){
    std::lock_guard<std::mutex> guard(sensor_mutex);
    // std::cout << "\nLatest Sensor Data";
    std::cout << "\n"
              << "Temperature: ";
    print_reading(m_readings.temperature, m_new_readings.temperature);
    std::cout << "\n"
              << "Humidity: ";
    print_reading(m_readings.humidity, m_new_readings.humidity);
    std::cout << "\n"
          << "Wind Speed: ";
    print_reading(m_readings.windspeed, m_new_readings.windspeed);
    std::cout << "\n";
}



void SensorData::print_single_statistic(Stats stat){
    std::cout << "Max: " << stat.max.value << ", " << stat.max.time_point << "\n"
              << "Min: " << stat.min.value << ", " << stat.min.time_point << "\n"
              << "Average: " << stat.average << "\n";
}

void SensorData::print_statistics(){
    std::lock_guard<std::mutex> guard(sensor_mutex);
    std::cout << "\nSensor Statistics\n"
              << std::chrono::system_clock::now() << "\n"

              << "Temperature: \n";
    print_single_statistic(m_statistics.temperature);

    std::cout << "Humidity: \n";
    print_single_statistic(m_statistics.humidity);

    std::cout << "Wind Speed: \n";
    print_single_statistic(m_statistics.windspeed);
}



void print_sensor_data() {
    {
        std::lock_guard<std::mutex> guard(sensor_mutex);
        std::cout << "STARTING WEATHER SENSORS - press q to QUIT\n";
    }    
    std::this_thread::sleep_for(100ms);
    int seconds{1};
    while (system_running) {
        if (seconds % 2 == 0) sensor_data::sensor.print_latest_readings();
        if (seconds % 10 == 0) sensor_data::sensor.print_statistics();
        for (int i = 0 ; i < 10 && system_running ; i++){
            std::this_thread::sleep_for(100ms);
        }
        seconds++;
    }
}




void quit_prompt(){
    std::string input;
    while (std::cin >> input){                
        if (input.at(0) == 'q') break;
    }    
    system_running = false;
    {
        std::lock_guard<std::mutex> guard(sensor_mutex);
        std::cout << "Quit program\n";
    }
}

int main() 
{

    std::thread temperature(sensor_temperature);
    std::thread relative_humidity(sensor_humidity);
    std::thread windspeed(sensor_windspeed);
    std::thread statistics(sensor_statistics);
    std::thread print_data(print_sensor_data);

    // std::thread user_prompt(quit_prompt);
    // user_prompt.join();

    temperature.join();
    relative_humidity.join();
    windspeed.join();
    system_running = false;
    statistics.join();
    print_data.join();


    return 0;
}
