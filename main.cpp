#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/filesystem.hpp>
#include <string>
#include <vector>
#include <iomanip>

#include <readline/readline.h>
#include <readline/history.h>
#include <wordexp.h>
#include <map>
#include "builtins.h"


std::vector<std::string> parser(std::istringstream &iss) {
    std::vector<std::string> v;
    std::string s;
    while (iss >> std::quoted(s))
        v.push_back(s);
    return v;
}

char *local_variable(char *argv_) {
    char *var_val = argv_;
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
            } else if (var_val[i] == '=') {
                middle = true;
                new_var[i] = '\0';
            } else {
                new_val[j] = var_val[i];
                j++;
            }
            new_val[j] = '\0';
        }
        setenv(new_var, new_val, 1);
    }
    return var_val;
}

void usage() {
    char string[] = "\t-h or --help this message\n     \t-A hidden characters in a hex code\n     \t./mycat file1.txt file2.txt ...\n";
    write(STDOUT_FILENO, string, sizeof(string));
}

int search_redirect(std::vector<std::string> argv) {
    for (int i = 0; i < argv.size(); i++) {
        if(argv[i].find(">") != std::string::npos || argv[i].find("<") != std::string::npos){
            return i;
        }
    }
    return 0;
}

void redirect(std::vector<std::string> argv){
    int red_pos = search_redirect(argv);
    if(red_pos){

    }
}

int main(int argc, char *argv[]) {
    int err_code = 0;
    int i = 1;
    std::vector<char *> child_envp;
    while (environ[i]) {
        int len = strlen(environ[i]);
        char *en_var = new char[len + 1];
        std::copy(environ[i], environ[i] + len + 1, en_var);
        child_envp.push_back(en_var);
        i++;
    }

    typedef std::function<int(char **)> func_t;

    std::map<std::string, func_t> m;

    auto merrno_bind = [&err_code](char **args) { return merrno(args, err_code); };
    auto mexport_bind = [&child_envp](char **args) { return mexport(args, child_envp); };
    m["merrno"] = merrno_bind;
    m["mpwd"] = mpwd;
    m["mcd"] = mcd;
    m["mexport"] = mexport_bind;
    m["mexit"] = mexit;
    m["mecho"] = mecho;


    boost::filesystem::path path;
    int optIdx, c = 1;
    int argc_ = 0;
    auto path_ptr = getenv("PATH");
    std::string path_var;
    if (path_ptr != nullptr)
        path_var = path_ptr;
    path_var += ":.";
    for (boost::filesystem::recursive_directory_iterator i("."), end; i != end; ++i) {
        if (is_directory(i->path())) {
            path_var += ":";
            path_var += (i->path()).string();
        }
    }
    setenv("PATH", path_var.c_str(), 1);

    while (true) {
        wordexp_t p;
        bool wordcard = false;


        std::string command;
        char *start = (char *) malloc(strlen(boost::filesystem::current_path().c_str()) + 3);
        strcpy(start, boost::filesystem::current_path().c_str());
        strcat(start, "$ ");
        command = readline(start);
        if (strlen(command.c_str()) > 0)
            add_history(command.c_str());
        std::istringstream iss(command);
        auto args = parser(iss);
        char **argv_ = (char **) malloc(args.size() * sizeof(char **));
        for (auto &i : args) {
            if (i != "") {
                if (i.find('*') != std::string::npos || i.find('?') != std::string::npos ||
                    i.find('[') != std::string::npos) {
                    wordcard = true;
                    wordexp(i.c_str(), &p, 0);
                    for (int j = 0; j < p.we_wordc; j++) {
                        if (access(p.we_wordv[j], F_OK) != 0) {
                            std::cout << p.we_wordv[j] << " is not a file or directory" << std::endl;
                            err_code = 1;
                            // here merror has to be not null !!!!!
                            break;
                        }
                        argv_[argc_] = p.we_wordv[j];
                        argc_++;
                    }
                } else {
                    argv_[argc_] = strdup(i.c_str());
                    argc_++;
                }
            }
        }
        argv_[argc_] = "\0";
        pid_t pid;

        auto iter = m.find(argv_[0]);
        if (iter == m.end()) {
            if (strchr(argv_[0], '=')) {
                local_variable(argv_[0]);
            } else if ((pid = fork()) < 0)
                std::cout << "Error" << std::endl;
            else if (pid == 0) {
                std::vector<const char *> arg_for_c;
                for (int i = 0; i < argc_; i++)
                    arg_for_c.push_back(argv_[i]);
                arg_for_c.push_back(nullptr);

                execvpe(argv_[0], const_cast<char *const *>(arg_for_c.data()), child_envp.data());
                std::cout << "-myshell: " << argv_[0] << ": command not found." << std::endl;
                break;
            } else {
                int status;
                waitpid(pid, &status, 0);
            }

        } else {
            auto func1 = *iter;
            err_code = (iter->second)(argv_);
        }
        delete[] argv_;
        argc_ = 0;
        if (wordcard) wordfree(&p);
        else wordcard = false;
    }
    return 0;
}