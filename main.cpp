#include <iostream>
#include <fstream>
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
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

typedef std::function<int(char **)> func_t;

bool found_char (char ** argv, char c){
    char **np = argv;
    while ( **np != '\0' ){
        if(strchr(*np++, '|'))
            return true;
    }
    return false;
}


void get_argument (char ** argv, size_t start_position, size_t end_position, char **args) {
    std::string result_string;
    bool found = false;
    if (start_position == end_position){
        args[0] = argv[start_position];
        args[1] = NULL;
    } else {
        for (int i = start_position; i <= end_position; i++) {
            if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "2>") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], "2>&1") == 0){
                args[i - start_position] = NULL;
                found = true;
            } else {
                if(found){
                    args[i - start_position] = NULL;
                } else
                    args[i - start_position] = argv[i];
            }
        }
        args[end_position + 1] = NULL;
    }
}

char * local_variable(char * argv_){
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

void findAllOccurances(std::vector<size_t> & vec, std::string data, std::string toSearch)
{
    size_t pos = data.find(toSearch);
    while( pos != std::string::npos) {
        vec.push_back(pos);
        pos = data.find(toSearch, pos + toSearch.size());
        std::cout << pos << std::endl;
    }
}

int pipe (char ** argv, int args) {
    std::vector<size_t> positions_vec;
    std::vector<size_t> redirect_positions_vec;
    int out;
    int pid, pid4;

    bool has_redirect = false;

    if (strcmp(argv[args - 2], ">") == 0 || strcmp(argv[args - 2], "2>") == 0 || strcmp(argv[args - 2], "2>&1") == 0) {
        has_redirect = true;
    }

    for (int i = 0; i < args; i++){
        if (strcmp(argv[i], "|") == 0) {
            positions_vec.push_back(i);
        } else if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "2>") == 0 || strcmp(argv[i], "2>&1" ) == 0 || strcmp(argv[i], "<" ) == 0 ) {
            redirect_positions_vec.push_back(i);
        }
    }

    if (positions_vec[0] == 0){
        std::cout << "Command " << argv[positions_vec[0]] << " not found." << std::endl;
        return -1;
    } else if (positions_vec.size() == 1){
        char **args_1 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
        char **args_2 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
        get_argument(argv, 0, positions_vec[0] - 1, args_1);
        get_argument(argv, positions_vec[0] + 1, args - 1, args_2);

        int pipefd[2];
        int pid3;
        int pid1, status;


        pipe(pipefd);
        pid1 = fork();
        if(pid1 == 0){
            pid = fork();
            if (pid == 0) {
                dup2(pipefd[0], 0);
                close(pipefd[1]);
                execvp(args_2[0], args_2);
            } else {
                dup2(pipefd[1], 1);
                close(pipefd[0]);
                execvp(args_1[0], args_1);
            }
        } else {
            wait(NULL);
            printf("");
        }
    } else if (positions_vec.size() == 2) {
        char **args_1 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
        char **args_2 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
        char **args_3 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
        char **args_4 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));

        get_argument(argv, 0, positions_vec[0] - 1, args_1);
        get_argument(argv, positions_vec[0] + 1, positions_vec[1] - 1, args_2);
        get_argument(argv, positions_vec[1] + 1, args - 1, args_3);


        int status;
        int i;

        int pipes[4];
        pipe(pipes); // sets up 1st pipe
        pipe(pipes + 2); // sets up 2nd pipe
        if (fork() == 0) {
            dup2(pipes[1], 1);
            close(pipes[0]);
            close(pipes[1]);
            close(pipes[2]);
            close(pipes[3]);
            execvp(args_1[0], args_1);
        }
        else {
            if (fork() == 0) {
                dup2(pipes[0], 0);
                dup2(pipes[3], 1);
                close(pipes[0]);
                close(pipes[1]);
                close(pipes[2]);
                close(pipes[3]);

                execvp(args_2[0], args_2);
            }
            else {
                if ((pid4 = fork()) == 0) {
                    dup2(pipes[2], 0);
                    close(pipes[0]);
                    close(pipes[1]);
                    close(pipes[2]);
                    close(pipes[3]);

                    if (has_redirect){
                        out = open(argv[redirect_positions_vec[0] + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                        if (strcmp(argv[redirect_positions_vec[0]], ">") == 0) {
                            dup2(out, 1);
                        }
                        else if(strcmp(argv[redirect_positions_vec[0]], "2>") == 0)
                            dup2(out, 2);
                        else if (strcmp(argv[redirect_positions_vec[0]], "2>&1") == 0) {
                            dup2(out, 1);
                            dup2(out, 2);
                        }
                    }

                    execvp(args_3[0], args_3);
                }
            }
        }
        close(pipes[0]);
        close(pipes[1]);
        close(pipes[2]);
        close(pipes[3]);

        close(out);
        for (i = 0; i < 3; i++)
            wait(&status);

    }
    return 0;
}

void redirect (char ** argv, int args) {
    std::vector<size_t> positions_vec;
    for (int i = 0; i < args; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "2>") == 0 || strcmp(argv[i], "2>&1" ) == 0 || strcmp(argv[i], "<" ) == 0 ) {
            positions_vec.push_back(i);
        }
    }
    pid_t pid;
    int status;
    if ((pid = fork()) < 0)
        std::cout << "Error" << std::endl;
    else if (pid == 0) {
        if (positions_vec.size() == 2){

        }
        else {
            int in, out;

            if (positions_vec[0] == 0){
                std::cout << "Command " << argv[positions_vec[0]] << " not found." << std::endl;
            }
            char **args_1 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
            char **args_2 = static_cast<char**>(malloc( (positions_vec[0] + 2) * sizeof(char*)));
            get_argument(argv, 0, positions_vec[0] - 1, args_1);
            if (args > 2) {
                get_argument(argv, positions_vec[0] + 1, args - 1, args_2);
                out = open(args_2[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            }

            char *grep_args[] = {"ls", NULL};

            const char* var1 = ">";
            const char* var2 = "2>";
            const char* var3 = "2>&1";
            const char* var4 = "<";

            if (strcmp(argv[positions_vec[0]], var1) == 0)
                dup2(out, 1);
            else if(strcmp(argv[positions_vec[0]], var2) == 0)
                dup2(out, 2);
            else if (strcmp(argv[positions_vec[0]], var3) == 0) {
                dup2(out, 1);
                dup2(out, 2);
            }
            else if (strcmp(argv[positions_vec[0]], var4) == 0) {
                if ((in = open(args_2[0], O_RDONLY)) < 0){
                    perror("Couldn't open input file");
                    _exit(0);
                }
                dup2(in, 0);
                close(in);
            }
            close(out);

            execvp(args_1[0], args_1);
            perror("execvp");
            _exit(1);

        }
    }
    else {
        while (!(wait(&status) == pid)) ;
    }

}

void run_command (char ** argv, int args, std::vector<char *> & child_envp) {
    int pid;
    bool is_background = false;
    if(strchr(argv[0],'='))
        local_variable(argv[0]);

    if ((pid = fork()) < 0)
        std::cout << "Error" << std::endl;
    else if (pid == 0) {
        std::vector<const char *> arg_for_c;
        for (int i = 0; i < args; i++) {
            if (strcmp(argv[i], "&") != 0)
                arg_for_c.push_back(argv[i]);
            else
                is_background = true;
        }
        arg_for_c.push_back(nullptr);

        execvpe(argv[0], const_cast<char *const *>(arg_for_c.data()), child_envp.data());
        std::cout << "-myshell: " << argv[0] << ": command not found." << std::endl;
    } else {
        if (strcmp(argv[args - 1], "&") != 0) {
            int status;
            waitpid(pid, &status, 0);
        } else if (strcmp(argv[args - 2], "&") == 0){
            std::cout << "-myshell: " << argv[args - 1] << ": command not found." << std::endl;
        }
    }
}

void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

void delchar(char *x,int a, int b) {
    if ((a+b-1) <= strlen(x)) {
        strcpy(&x[b-1],&x[a+b-1]);
        puts(x);
    }
}

void $_command (char ** argv, int args,  std::vector<char *> & child_envp) {
    std::vector<size_t> positions_vec;
    std::string arg2;
    std::vector<const char *> arg_for_c;
    std::vector<const char *> arg_for_c_1;

    for (int i = 0; i < args; i++){
        if ( strstr(argv[i], "$(") && strstr(argv[i], ")") ){
            positions_vec.push_back(i);
            removeChar(argv[i], '$');
            removeChar(argv[i], '(');
            removeChar(argv[i], ')');
            char *token = strtok(argv[i], " ");
            while (token) {
                arg_for_c.push_back(token);
                token = strtok(NULL, " ");
            }
            FILE *fp;
            int  stdout_bk;
            stdout_bk = dup(fileno(stdout));
            fp=fopen("temp.txt","w");//file out, after read from file
            dup2(fileno(fp), fileno(stdout));
            run_command(const_cast<char **>(arg_for_c.data()), arg_for_c.size(), child_envp);
            fflush(stdout);
            fclose(fp);
            dup2(stdout_bk, fileno(stdout));
            const char* filename = "temp.txt";
            std::ifstream file(filename);
            if (file.is_open()) {
                std::string str = "'";
                std::string line;
                while (getline(file, line))
                    str = str + line + "\n";
                str = str + "'";
                arg_for_c_1.push_back(str.c_str());
            } else std::cout << "Couldn't open a file" << std::endl;
            file.close();
        } else {
            arg_for_c_1.push_back(argv[i]);
        }
    }
    run_command(const_cast<char **>(arg_for_c_1.data()), arg_for_c_1.size(), child_envp);
}


void do_all_stuff (std::vector<std::string> & args, std::map<std::string, func_t> &m, wordexp_t & p, std::vector<char *> & child_envp, bool wordcard, bool is_pipe, bool is_redirect, bool is_command, bool is_background, int err_code) {
    int argc_ = 0;
    char ** argv_ = (char **) malloc(args.size() * sizeof(char **));
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
                        break;
                    }
                    argv_[argc_] = p.we_wordv[j];
                    argc_++;
                }
            } else if (i.find('|') != std::string::npos) {
                argv_[argc_] = strdup(i.c_str());
                argc_++;
                is_pipe = true;
            } else if (i.find('>') != std::string::npos || i.find('<') != std::string::npos) {
                argv_[argc_] = strdup(i.c_str());
                argc_++;
                is_redirect = true;
            } else if (i.find('$(') != std::string::npos) {
                if (i.find('$(') != std::string::npos && i.find(')') != std::string::npos)
                    argv_[argc_] = strdup(i.c_str());
                else if (i.find('$(') != std::string::npos && (*(&i + 1)).find(')') != std::string::npos){
                    char * str3 = (char *) malloc(2 + strlen(i.c_str())+ strlen((*(&i + 1)).c_str()) );
                    strcpy(str3, i.c_str());
                    strcat(str3, " ");
                    strcat(str3, (*(&i + 1)).c_str());
                    argv_[argc_] = str3;
                }
                argc_++;
                is_command = true;
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
        if(strchr(argv_[0],'=')){
            local_variable(argv_[0]);
        }
        else if (is_pipe) {
            pipe(argv_, argc_);
        } else if (is_redirect){
            redirect(argv_, argc_);
        } else if (is_command){
            $_command (argv_, argc_, child_envp);
        } else {
            run_command(argv_, argc_, child_envp);
        }
    } else {
        auto func1 = *iter;
        err_code = (iter->second)(argv_);
    }
    delete[] argv_;
    argc_ = 0;
    if (wordcard) wordfree(&p);
    else wordcard = false;
    is_background = false;
}


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
            if(result == nullptr){
                return -1;
            }
            int len_ = strlen(result);
            int len = strlen(var_val);
            char * new_var = new char[len + len_ + 2];
            for (int i = 0, j = 0; i < len + len_ + 1; i++) {
                if (i < len) {
                    new_var[i] = var_val[i];
                } else if (i == len) {
                    new_var[i] = '=';
                } else {
                    new_var[i] = result[j];
                    j++;
                }
                new_var[len + len_ + 1] = '\0';
            }
            env_vars.push_back(new_var);
        }
        return 0;
    }
    catch(std::exception &e){
        return -1;
    }
}

std::vector<std::string> parser(std::istringstream &iss) {
    std::vector<std::string> v;
    std::string s;
    while (iss >> std::quoted(s))
        v.push_back(s);
    return v;
}

void usage() {
    char string[] = "\t-h or --help this message\n     \t-A hidden characters in a hex code\n     \t./mycat file1.txt file2.txt ...\n";
    write(STDOUT_FILENO, string, sizeof(string));
}

int main(int argc, char *argv[]) {
    int err_code = 0;
    int i = 1;
    std::vector<char *> child_envp;
    while (environ[i]){
        int len = strlen(environ[i]);
        char * en_var = new char[len+1];
        std::copy(environ[i],environ[i]+len+1, en_var);
        child_envp.push_back(en_var);
        i++;
    }

    std::map<std::string, func_t> m;

    auto merrno_bind = [&err_code](char **args) { return merrno(args, err_code); };
    auto mexport_bind = [&child_envp](char ** args) { return mexport(args, child_envp); };
    m["merrno"] = merrno_bind;
    m["mpwd"] = mpwd;
    m["mcd"] = mcd;
    m["mexport"] = mexport_bind;
    m["mexit"] = mexit;
    m["mecho"] = mecho;

    boost::filesystem::path path;
    int optIdx, c = 1;
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
        bool is_pipe = false;
        bool is_redirect = false;
        bool is_pipe_and_redirect = false;
        bool is_background = false;
        bool is_command = false;
        bool is_msh = false;
        bool ignore_string = false;
        std::string line;

        std::string command;
        char *start = (char *) malloc(strlen(boost::filesystem::current_path().c_str()) + 3);
        strcpy(start, boost::filesystem::current_path().c_str());
        strcat(start, "$ ");
        command = readline(start);
        if (strlen(command.c_str()) > 0)
            add_history(command.c_str());
        std::istringstream iss(command);
        auto args = parser(iss);

        if (args.size() == 1 && args[0].find(".msh") != std::string::npos && args[0][0] != '.') {
            const char* filename = args[0].c_str();
            std::ifstream file(filename);
            if (file.is_open()) {
                std::string line;
                while (getline(file, line)) {
                    int k = 0;
                    if (line[0] == '#')
                        ignore_string = true;
                    else {
                        args.resize(line.size());
                        std::stringstream ss (line);
                        std::string item;
                        while (getline (ss, item, ' ')) {
                            args[k] = item;
                            k++;
                        }
                        do_all_stuff (args, m, p,child_envp, wordcard, is_pipe, is_redirect, is_command, is_background, err_code);
                        args.clear();
                    }
                }
                file.close();
            }
        } else if (args.size() == 1 && args[0].find(".msh") != std::string::npos && args[0][0] == '.') {
            char cwd[100];
            args[0].erase (args[0].begin());
            getcwd(cwd, sizeof(cwd));
            std::string path_var_2;
            auto path_ptr_2 = getenv("PATH");
            if (path_ptr_2 != nullptr)
                path_var_2 = path_ptr_2;
            path_var_2 += std::string(cwd) + "/" + args[0];
            setenv("PATH", path_var_2.c_str(), 1);
        }
        else do_all_stuff (args, m, p, child_envp, wordcard, is_pipe, is_redirect, is_command, is_background, err_code);
    }
    return 0;
}