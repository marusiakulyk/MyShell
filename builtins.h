//
// Created by marusia on 18.11.19.
//

#ifndef LAB3_BUILTINS_H
#define LAB3_BUILTINS_H

#include <vector>
#include "builtins.cpp"

int merrno(char **args, int &err_code);
int mpwd(char **args);
int mcd(char **args);
int mexit(char  ** args);
int mecho(char ** text);
int mexport(char ** args, std::vector<char *> & env_vars);


#endif //LAB3_BUILTINS_H
