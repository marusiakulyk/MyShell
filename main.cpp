#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

std::vector<std::string> parser(std::string& str, const std::string& delims = " "){
    std::vector<std::string> args;
    boost::split(args, str, boost::is_any_of(delims));
    return args;
};

int main(void) {
    while(true) {
        std::string command;
        std::cout << boost::filesystem::current_path().string() << "$";
        std::getline(std::cin, command);
        if(command == "") break;
        auto args = parser(command);
        for(auto &i : args){
            std::cout<< i<< "\n";
        }
    }
}