#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "cli.h"

using namespace std;

void print_usage(char* bin_name, string mode, string help, const map<string, string, cmp>& arg_help) {
    cout << help << " Example usage is: " << endl << endl;
    cout << "\t" << bin_name << " " << mode << " ";
    for (auto& arg : arg_help) {
        cout << "[" << arg.first << "]" << " ";
    }

    cout << endl << endl;
    for (auto& arg : arg_help) {
        cout << "\t" << arg.first << ": " << arg.second << endl;
    }
}

bool verify_args(int argc, char* bin_name, string mode, string help, const map<string, string, cmp>& arg_help) {
    if (argc != arg_help.size() + 2) {
        cout << "Incorrect usage of the " << mode << " mode. ";
        print_usage(bin_name, mode, help, arg_help);

        return false;
    }

    return true;
}

void display_help(char* bin_name, vector<string> modes, map<string, string> helps, map<string, map<string, string, cmp>> args) {
    cout << "There are four modes to run." << endl << endl;

    for (auto& m : modes) {
        print_usage(bin_name, m, helps[m], args[m]);
        cout << endl;
    }
}