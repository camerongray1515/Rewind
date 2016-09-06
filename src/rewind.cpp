#include <iostream>
#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <string.h>

std::string filename = "/tmp/pending-rollback.rewind";

void run(char change_command[], char rollback_command[], int timeout);
void keep();
void print_usage(char *argv[]);
void create_file(char change_command[]);
bool file_exists();
void delete_file();

int main(int argc, char *argv[]) {
    int timeout = 30;

    int opt;
    while((opt = getopt(argc, argv, "t:")) != -1) {
        switch(opt) {
            case 't':
                timeout = atoi(optarg);
                break;
            case '?':
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (argc < 2) {
        std::cout << "You must supply an action to be performed" << std::endl;
        print_usage(argv);
        return 0;
    }

    int option_base = optind;
    std::string mode(argv[option_base]);

    if (mode == "run") {
        char *change_command = argv[option_base+1];
        char *rollback_command = argv[option_base+2];
        if (argc < 4 || strlen(change_command) == 0 || strlen(rollback_command) == 0) {
            std::cout << "Must provide both a change command and a rollback command" << std::endl;
            print_usage(argv);
            return 1;
        }

        if (timeout < 1) {
            std::cout << "Timeout must be at least 1" << std::endl;
            print_usage(argv);
            return 1;
        }

        run(change_command, rollback_command, timeout);
    } else if(mode == "keep") {
        keep();
    } else {
        print_usage(argv);
    }

    return 0;
}

void print_usage(char *argv[]) {
    std::cout << "Usage:" << std::endl;
    std::cout << "  Run a new command:\t" << argv[0] << " run [options] <change command> <rollback command>" << std::endl;
    std::cout << "  Stop rollback:\t" << argv[0] << " keep" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -t  -  Number of seconds to wait before executing the rollback command.  Defaults to 30 seconds." << std::endl;
}

void run(char change_command[], char rollback_command[], int timeout) {
    create_file(change_command);
    system(change_command);
    daemon(1, 0); // Do not change CWD, redirect output to /dev/null
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
