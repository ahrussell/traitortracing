/*
 * cli.h
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

// ensures map<string, string, cmp> will keep original ordering
struct cmp {
    bool operator()(const string& a, const string& b) const {
        return 1;
    }
};

void print_usage(char* bin_name, string mode, string help, const map<string, string, cmp>& arg_help);
bool verify_args(int argc, char* bin_name, string mode, string help, const map<string, string, cmp>& arg_help);
void display_help(char* bin_name, vector<string> modes, map<string, string> helps, map<string, map<string, string, cmp>> args);