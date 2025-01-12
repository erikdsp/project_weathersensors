#include "SensorData.h"

extern std::mutex sensor_mutex;

void SensorData::store_new_reading(double reading, std::vector<TimeDouble>& readings) {
    std::lock_guard<std::mutex> guard(sensor_mutex);
    readings.emplace_back(std::chrono::system_clock::now(), reading);
}

/**
 *  Public setter functions 
 */
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

