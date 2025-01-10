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

// how to get a time stamp
// std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
// auto t1 = std::chrono::steady_clock::now();

std::mutex sensor_mutex;
std::atomic_bool system_running { true };  


// storing time_point and double pairs
struct TimeDouble {
    std::chrono::system_clock::time_point time_point;
    double value;
};

struct SensorReadings {
    using time_point = std::chrono::system_clock::time_point;
    std::vector<TimeDouble> temperature;
    std::vector<TimeDouble> humidity;
    std::vector<TimeDouble> windspeed;
    // std::map<time_point, double> temperature;
    // std::map<time_point, double> humidity;
    // std::map<time_point, double> windspeed;
};

// storing each sensors statistics
struct Stats {
    TimeDouble max;
    TimeDouble min;
    double average;
};

// container for all statistics
struct SensorStatistics {
    Stats temperature;
    Stats humidity;
    Stats windspeed;
};

// global object to store data
namespace sensor_data {
    SensorReadings new_readings;
    SensorReadings readings;
    SensorStatistics statistics;
}

// temperatur, luftfuktighet och vindhastighet
void sensor_temperature()
{
    // initialize sensor data
    const double min_temp { -15.0 };
    const double max_temp { 30.0 };
    static std::mt19937 random { std::random_device{}() };
    static std::uniform_real_distribution<> temp_distrib { min_temp, max_temp };
    static std::uniform_real_distribution<> tempfluct_distrib { -0.2, 0.2 };
    double temperature { temp_distrib(random) };
    

    for (int i = 0 ; i < 10 ; ++i)
    {
        // generate sensor data
        double fluct { tempfluct_distrib(random) };
        double new_temp = temperature += fluct;
        // check that temp does not exceed bounds
        if ( new_temp < min_temp || new_temp > max_temp ) {
            new_temp = temperature -= fluct;
        }
        temperature = new_temp;
        {
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.temperature.emplace_back(std::chrono::system_clock::now(), temperature);
            std::cout << temperature << "\n";
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

    for (int i = 0 ; i < 10 ; ++i) {
        // generate sensor data
        double fluct { humfluct_distrib(random) };
        double new_hum = relative_humidity += fluct;
        // check that humidity does not exceed bounds
        if ( new_hum < min_humidity || new_hum > max_humidity ) {
            new_hum = relative_humidity -= fluct;
        }
        relative_humidity = new_hum;
        {
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.humidity.emplace_back(std::chrono::system_clock::now(), relative_humidity);
            std::cout << "\t" << relative_humidity << "\n";
        }
        std::this_thread::sleep_for(500ms);
    }
}

void sensor_windspeed()
{
    // max uppmätt är 47 m/s
    // initialize sensor data
    const double min_windspeed { 0.0 };
    const double max_windspeed { 25.0 };
    static std::mt19937 random { std::random_device{}() };
    static std::uniform_real_distribution<> windspeed_distrib { min_windspeed, max_windspeed };
    static std::uniform_real_distribution<> windspeedfluct_distrib { -0.5, 0.5 };
    double windspeed { windspeed_distrib(random) };
    
    for (int i = 0 ; i < 10 ; ++i)
    {
        // generate sensor data
        double fluct { windspeedfluct_distrib(random) };
        double new_windspeed = windspeed += fluct;
        // check that windspeed does not exceed bounds
        if ( new_windspeed < min_windspeed || new_windspeed > max_windspeed ) {
            new_windspeed = windspeed -= fluct;
        }
        windspeed = new_windspeed;
        {
            std::lock_guard<std::mutex> guard(sensor_mutex);
            sensor_data::new_readings.windspeed.emplace_back(std::chrono::system_clock::now(), windspeed);
            std::cout << "\t\t" << windspeed << "\n";
        }
        std::this_thread::sleep_for(500ms);
    }
}

void calculate_statistics(Stats& stat, bool& first_reading,
    std::vector<TimeDouble> readings,
    std::vector<TimeDouble> new_readings) {

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
        std::this_thread::sleep_for(5s);
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

            // sensor_data::readings.humidity.insert_range(std::move(sensor_data::new_readings.temperature));
            // sensor_data::readings.windspeed.insert_range(std::move(sensor_data::new_readings.temperature));
            // clear new_readings
            // sensor_data::new_readings.humidity.clear();
            // sensor_data::new_readings.windspeed.clear();
        }
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
    std::cout << "Temp\tHumid\tWind\n";
    std::thread temperature(sensor_temperature);
    std::thread relative_humidity(sensor_humidity);
    std::thread windspeed(sensor_windspeed);

    // std::thread user_prompt(quit_prompt);
    // user_prompt.join();

    temperature.join();
    relative_humidity.join();
    windspeed.join();



    // test printing that data is saved
    std::cout << "\nTemperature:\n";
    for (auto& temp : sensor_data::new_readings.temperature) {
        std::cout << std::chrono::system_clock::to_time_t(temp.time_point) << "\t";
        std::cout << temp.value << "\n";
    }

    std::cout << "\nHumidity:\t";
    for (auto& hum : sensor_data::new_readings.humidity) {
        std::cout << hum.value << "\t";
    }

    std::cout << "\nWind Speed:\t";
    for (auto& ws : sensor_data::new_readings.windspeed) {
        std::cout << ws.value << "\t";
    }
    std::cout << "\n";


    return 0;
}
