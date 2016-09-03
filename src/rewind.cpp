#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <boost/program_options.hpp>
#include <ctime>
#include <iomanip>

std::string filename = "/tmp/pending-rollback.rewind";

namespace po = boost::program_options;

void run(char change_command[], char rollback_command[], int timeout);
void keep();
void print_usage(char *argv[], po::options_description desc);
void create_file(char change_command[]);
bool file_exists();
void delete_file();

int main(int argc, char *argv[]) {
    int timeout = 30;

    po::options_description desc("Allowed options");
    desc.add_options()("timeout,t", po::value<int>(&timeout)->default_value(timeout),
            "Timeout before rollback command is executed");
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    } catch (po::unknown_option e) {
        std::cout << "Unknown option" << std::endl;
        print_usage(argv, desc);
        return 0;
    } catch (po::invalid_option_value e) {
        std::cout << "Invalid value for option" << std::endl;
        print_usage(argv, desc);
        return 0;
    }
    po::notify(vm);

    if (argc < 2) {
        std::cout << "You must supply an action to be performed" << std::endl;
        print_usage(argv, desc);
        return 0;
    }

    std::string mode(argv[1]);

    if (mode == "run") {
        char *change_command = argv[2];
        char *rollback_command = argv[3];
        if (argc < 4 || strlen(change_command) == 0 || strlen(rollback_command) == 0) {
            std::cout << "Must provide both a change command and a rollback command" << std::endl;
            print_usage(argv, desc);
            return 1;
        }

        if (timeout < 1) {
            std::cout << "Timeout must be at least 1" << std::endl;
            print_usage(argv, desc);
            return 1;
        }

        run(change_command, rollback_command, timeout);
    } else if(mode == "keep") {
        keep();
    } else {
        print_usage(argv, desc);
    }

    return 0;
}

void print_usage(char *argv[], po::options_description desc) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Run a new command:\t" << argv[0] << " run <change command> <rollback command> [options]" << std::endl;
    std::cout << "  Stop rollback:\t" << argv[0] << " keep" << std::endl;
    std::cout << desc << std::endl;
}

void run(char change_command[], char rollback_command[], int timeout) {
    create_file(change_command);
    system(change_command);
    daemon(1, 0);
    sleep(timeout);

    if (file_exists()) {
        system(rollback_command);
        delete_file();
    }
}

void keep() {
    delete_file();
    std::cout << "Rollback aborted, changes will be kept!" << std::endl;
}

void create_file(char change_command[]) {
    // Check if the file from an old run exists, if so ask the user to confirm
    // deletion
    std::ifstream inf(filename);
    if (inf.good()) {
        std::string timestamp;
        std::string command;
        std::getline(inf, timestamp);
        std::getline(inf, command);

        std::cout << "Detected a pending rollback for command \"" << command;
        std::cout << "\" started at " << timestamp << std::endl;

        std::cout << "This may be an error, you may cancel the rollback and ";
        std::cout << "proceed with this run however if the previous run is ";
        std::cout << "still active then it will not be rolled back." << std::endl;

        std::cout << "Cancel previous rollback and start this run? (y/N): " << std::flush;
        char response = std::cin.get();
        if (response != 'y' && response != 'Y') {
            std::cout << "Aborting run!" << std::endl;
            return;
        }
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ofstream outf(filename);
    outf << std::put_time(&tm, "%Y-%m-%d %H-%M-%S") << std::endl;
    outf << change_command << std::endl;
}

void delete_file() {
    std::remove(filename.c_str());
}

bool file_exists() {
    std::ifstream f(filename);
    return f.good();
}
