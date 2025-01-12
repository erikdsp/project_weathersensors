#include "threads.h"

extern std::mutex sensor_mutex;
extern std::atomic_bool system_running;

namespace sensor_data {
    extern SensorData sensor;
}

void sensor_temperature()
{
    DataGenerator temperature_generator ( -15, 30, -0.2, 0.2 );
    double temperature { temperature_generator.get_initial_value() };

    while (system_running) {
        temperature = temperature_generator.get_new_value();
        sensor_data::sensor.store_temperature_reading(temperature);
        std::this_thread::sleep_for(500ms);
    }

}

void sensor_humidity()
{
    DataGenerator humidity_generator ( 55.0, 100.0, -0.1, 0.1 );
    double humidity { humidity_generator.get_initial_value() };

    while (system_running) {
        humidity = humidity_generator.get_new_value();
        sensor_data::sensor.store_humidity_reading(humidity);
        std::this_thread::sleep_for(500ms);
    }
}

void sensor_windspeed()
{
    DataGenerator windspeed_generator ( 0.0, 25.0, -0.5, 0.5 );
    double windspeed { windspeed_generator.get_initial_value() };

   while (system_running) {
        windspeed = windspeed_generator.get_new_value();
        sensor_data::sensor.store_windspeed_reading(windspeed);
        std::this_thread::sleep_for(500ms);
    }    
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


void print_sensor_data() {
    {
        std::lock_guard<std::mutex> guard(sensor_mutex);
        std::cout << "STARTING SENSOR MONITORING - press q to QUIT\n";
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
}