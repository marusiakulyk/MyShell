#include <cstring>
#include <iostream>
#include <boost/filesystem/operations.hpp>

int merrno(char **args, int &err_code) {
    if (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
        std::cout << "Returns errno";
    } else if (args[1] == "\0") {
        std::cout << err_code << std::endl;
        return 0;
    }
    return -1;
}

int mpwd(char **args) {
    if (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
        std::cout << "Returns current path";
    } else if (args[1] == "\0") {
        std::cout << boost::filesystem::current_path().string() << std::endl;
        return 0;
    }
    return -1;
}

int mcd(char **args) {
    if (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
        std::cout << "Changes directory" << "\n";
    } else if (args[1] == "\0") {}
    else if (args[2] != "\0") {
        return -1;
    } else if (args[1] != "\0") {
        auto *path = realpath(args[1], nullptr);
        chdir(path);
    }
    return 0;
}
int mexit(char  ** args){
    if (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
        std::cout << "Exits myshell";
    } else if (args[1] == "\0") {
        std::exit(0);
    } else {
        std::exit(std::stoi(args[1]));
    }
}



int mecho(char ** text){
    if (strcmp(text[1], "-h") == 0 || strcmp(text[1], "--help") == 0) {
        std::cout << "Prints out the string passed by the user or value of the passed environment variable";
    } else if (text[1] == "\0") {
        std::cout << std::endl;
    }
    int i = 1;
    while(text[i] != "\0"){
        if (strchr(text[i],('$'))){
            if (const char *env_p = std::getenv(text[i]+1)){
                if (env_p == nullptr){
                    std::cout << std::endl;
                }
                else{std::cout << env_p << std::endl;}
            }
        } else if (strlen(text[i])) {
            std::cout << text[i] << std::endl;
        }
        i++;
    }
    return 0;
}



int mexport(char ** args, std::vector<char *> & env_vars){
    if (strcmp(args[1], "-h") == 0 || strcmp(args[1], "--help") == 0) {
        std::cout << "Exports the variable to the environment variables of current process";
    } else if (args[1] == "\0") {
        return 1;
    }
    try {
        char *var_val = args[1];
        char *p;
        p = strchr(var_val, '=');
        if (p) {
            int len = strlen(var_val);
            bool middle = false;
            char new_var[len];
            char new_val[len];
            for (int i = 0, j = 0; i < len; i++) {
                if (var_val[i] != '=' && !middle) {
                    new_var[i] = var_val[i];
                } else if(var_val[i] == '='){
                    middle = true;
                    new_var[i] = '\0';
                } else{
                    new_val[j] = var_val[i];
                    j++;
                } new_val[j] = '\0';
            }
            env_vars.push_back(var_val);
            setenv(new_var,new_val,1);
        } else {
            const char *result = getenv(var_val);
            int len_ = strlen(result);
            int len = strlen(var_val);
            char new_var[len + len_ + 1];
            for (int i = 0, j = 0; i < len + len_ + 1; i++) {
                if (i < len) {
                    new_var[i] = var_val[i];
                } else if (i == len) {
                    new_var[i] = '=';
                } else {
                    new_var[i] = result[j];
                    j++;
                }
            }
            env_vars.push_back(new_var);
        }
        return 0;
    }
    catch(std::exception &e){
        return -1;
    }
}
