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
 
/** Global object to store data
 *  Sensor readings are stored in @param new_readings 
 *  Statistics are calculated from new_readings, stored in @param statistics
 *  then new_readings is moved to @param readings 
 */
namespace sensor_data {
    SensorReadings new_readings;
    SensorReadings readings;
    SensorStatistics statistics;
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
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.temperature.emplace_back(std::chrono::system_clock::now(), temperature);
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
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.humidity.emplace_back(std::chrono::system_clock::now(), relative_humidity);
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
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.windspeed.emplace_back(std::chrono::system_clock::now(), windspeed);
        }
        std::this_thread::sleep_for(500ms);
    }
}

/**
 *  Calculates Max, Min and Average
 *  @param first_reading    If true set max and min, then update first_reading to false
 *  @param sum              Calculate sum of readings and new_readings to get average
 *  @param stat             Stats variable gets updated by the function
 */
void calculate_statistics(Stats& stat, bool& first_reading,
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

/**
 *  Move from new_readings to readings 
 */
void move_sensor_data( std::vector<TimeDouble>& readings, std::vector<TimeDouble>& new_readings ) {
    // move data into readings
    readings.insert(readings.end(), 
                    std::make_move_iterator(new_readings.begin()), 
                    std::make_move_iterator(new_readings.end()) );
    // clear new_readings
    new_readings.clear();
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
            calculate_statistics(sensor_data::statistics.temperature, first_temperature, 
                sensor_data::readings.temperature, sensor_data::new_readings.temperature);

            calculate_statistics(sensor_data::statistics.humidity, first_humidity, 
                sensor_data::readings.humidity, sensor_data::new_readings.humidity);

            calculate_statistics(sensor_data::statistics.windspeed, first_windspeed, 
                sensor_data::readings.windspeed, sensor_data::new_readings.windspeed);

            move_sensor_data(sensor_data::readings.temperature, sensor_data::new_readings.temperature);
            move_sensor_data(sensor_data::readings.humidity, sensor_data::new_readings.humidity);
            move_sensor_data(sensor_data::readings.windspeed, sensor_data::new_readings.windspeed);
        }
    }
}

void print_reading( const std::vector<TimeDouble>& readings, const std::vector<TimeDouble>& new_readings) {
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

void print_latest_readings(){
    std::lock_guard<std::mutex> guard(sensor_mutex);
    // std::cout << "\nLatest Sensor Data";
    std::cout << "\n"
              << "Temperature: ";
    print_reading(sensor_data::readings.temperature, sensor_data::new_readings.temperature);
    std::cout << "\n"
              << "Humidity: ";
    print_reading(sensor_data::readings.humidity, sensor_data::new_readings.humidity);
    std::cout << "\n"
          << "Wind Speed: ";
    print_reading(sensor_data::readings.windspeed, sensor_data::new_readings.windspeed);
    std::cout << "\n";
}

void print_single_statistic(Stats stat){
    std::cout << "Max: " << stat.max.value << ", " << stat.max.time_point << "\n"
              << "Min: " << stat.min.value << ", " << stat.min.time_point << "\n"
              << "Average: " << stat.average << "\n";
}

void print_statistics(){
    std::lock_guard<std::mutex> guard(sensor_mutex);
    std::cout << "\nSensor Statistics\n"
              << std::chrono::system_clock::now() << "\n"

              << "Temperature: \n";
    print_single_statistic(sensor_data::statistics.temperature);

    std::cout << "Humidity: \n";
    print_single_statistic(sensor_data::statistics.humidity);

    std::cout << "Wind Speed: \n";
    print_single_statistic(sensor_data::statistics.windspeed);
}

void print_sensor_data() {
    std::cout << "STARTING WEATHER SENSORS - press q to QUIT\n";
    std::this_thread::sleep_for(100ms);
    int seconds{1};
    while (system_running) {
        if (seconds % 2 == 0) print_latest_readings();
        if (seconds % 10 == 0) print_statistics();
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
