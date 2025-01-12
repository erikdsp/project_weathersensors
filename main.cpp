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


void sensor_temperature()
{
    // initialize sensor data
    const double min_temp { -15.0 };
    const double max_temp { 30.0 };
    static std::mt19937 random { std::random_device{}() };
    static std::uniform_real_distribution<> temp_distrib { min_temp, max_temp };
    static std::uniform_real_distribution<> tempfluct_distrib { -0.2, 0.2 };
    double temperature { temp_distrib(random) };
    

    for (int i = 0 ; i < 50 ; ++i)
    {
        // generate sensor data
        double fluct { tempfluct_distrib(random) };
        double new_temp = temperature += fluct;
        // check that temp does not exceed bounds
        if ( new_temp < min_temp || new_temp > max_temp ) {
            new_temp = temperature -= fluct;
        }
        temperature = new_temp;
        // store sensor data
        {
            sensor_data::sensor.store_temperature_reading(temperature);
        }
        std::this_thread::sleep_for(500ms);
    }

}

void sensor_humidity()
{
    // initialize sensor data
    const double min_humidity { 55.0 };
    const double max_humidity { 100.0 };
    static std::mt19937 random { std::random_device{}() };
    static std::uniform_real_distribution<> humidity_distrib { min_humidity, max_humidity };
    static std::uniform_real_distribution<> humfluct_distrib { -0.1, 0.1 };
    double relative_humidity { humidity_distrib(random) };

    for (int i = 0 ; i < 50 ; ++i) {
        // generate sensor data
        double fluct { humfluct_distrib(random) };
        double new_hum = relative_humidity += fluct;
        // check that humidity does not exceed bounds
        if ( new_hum < min_humidity || new_hum > max_humidity ) {
            new_hum = relative_humidity -= fluct;
        }
        relative_humidity = new_hum;
        // store sensor data
        {
            sensor_data::sensor.store_humidity_reading(relative_humidity);
        }
        std::this_thread::sleep_for(500ms);
    }
}

void sensor_windspeed()
{
    // initialize sensor data
    const double min_windspeed { 0.0 };
    const double max_windspeed { 25.0 };
    static std::mt19937 random { std::random_device{}() };
    static std::uniform_real_distribution<> windspeed_distrib { min_windspeed, max_windspeed };
    static std::uniform_real_distribution<> windspeedfluct_distrib { -0.5, 0.5 };
    double windspeed { windspeed_distrib(random) };
    
    for (int i = 0 ; i < 50 ; ++i)
    {
        // generate sensor data
        double fluct { windspeedfluct_distrib(random) };
        double new_windspeed = windspeed += fluct;
        // check that windspeed does not exceed bounds
        if ( new_windspeed < min_windspeed || new_windspeed > max_windspeed ) {
            new_windspeed = windspeed -= fluct;
        }
        windspeed = new_windspeed;
        // store sensor data
        {
            sensor_data::sensor.store_windspeed_reading(windspeed);
        }
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
