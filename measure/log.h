#pragma

#include <iostream>
#include <string>



inline void log_info(const std::string& str){
    std::cout << "[INFO]: " << str << std::endl;
}

inline void log_warn(const std::string& str){
    std::cout << "[WARN]: " << str << std::endl;
}

inline void log_error(const std::string& str){
    std::cerr << "[ERROR]: " << str << std::endl;
}
