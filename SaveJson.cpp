#include "SaveJson.h"

namespace sensor_data {
    extern SensorData sensor;
}

bool file_exists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string get_current_date_cstyle() {
    std::time_t t = std::time(nullptr);
    char time_string[100]; 
    if (std::strftime(time_string, sizeof(time_string), "%d%b%Y", std::localtime(&t))){
        return time_string;
    }
    else return "date_error";
}

std::string generate_free_json_filename(const std::string& filename) {
    const std::string namebase { filename + "-" + get_current_date_cstyle() };
    std::string name_to_check { namebase + ".json" };
    int suffix { 0 };
    while (1) {
        if (!file_exists(name_to_check)) return name_to_check;
        suffix++;
        name_to_check = namebase + "-" + std::to_string(suffix) + ".json";
    }
    return " ";    
}

std::string save_sensordata_to_json(const std::string& filename, const SensorData& data){
    json json_data { data.construct_json_object() };
    std::string filename_json { generate_free_json_filename(filename) };
    // open file stream
    std::ofstream o(filename_json);
    // save data to file
    o << std::setw(3) << json_data << std::endl;
    return std::move(filename_json);
}