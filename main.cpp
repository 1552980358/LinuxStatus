#include <sys/stat.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unistd.h>

using std::string;
using std::cout;
using std::to_string;
using std::ifstream;
using std::stringstream;
using std::setprecision;
using std::fixed;
using std::endl;
using std::endl;

void get_cpu_usage_raw(unsigned long long &, unsigned long long &, unsigned long long &, unsigned long long &);

void show_thermal_temp();

void show_cpu_usage(const unsigned long long &, const unsigned long long &, const unsigned long long &, const unsigned long long &);

void show_memory_info();
void show_virtual_memory_info(const char [1024]);
void show_physical_memory_info(const char [1024]);

/** File operation **/
int is_dir_exists(const string &);

int is_dir_exists(const char *);

int is_file_exists(const string &);

int is_file_exists(const char *);

string read_file(const string &);

/** data type conversion **/
int str_to_int(const string &);

long kilobyte_to_megabyte(const long &);

int main() {

    cout << "Please wait a second for fetching data..." << endl
         << endl;
    unsigned long long last_total_user, last_total_user_low, last_total_sys, last_total_idle;
    get_cpu_usage_raw(last_total_user, last_total_user_low, last_total_sys, last_total_idle);

    sleep(1);

    cout << "CPU: " << endl;
    show_cpu_usage(last_total_user, last_total_user_low, last_total_sys, last_total_idle);

    cout << "CPU Sensors Temperature: " << endl;
    show_thermal_temp();

    cout << endl
         << "Memory: " << endl;
    show_memory_info();

    cout << endl;
    return 0;
}

int get_no_of_thermal_zones() {
    int i = 0;
    while (true) {
        if (is_dir_exists("/sys/class/thermal/thermal_zone" + to_string(i))) {
            i++;
            continue;
        }
        return i;
    }
}

void show_thermal_temp() {
    int no_of_thermal_zones = get_no_of_thermal_zones();
    string str;
    for (int i = 0; i < no_of_thermal_zones; ++i) {
        cout << " - "
             << read_file("/sys/class/thermal/thermal_zone" + to_string(i) + "/" + "type")
             << ": "
             << fixed << setprecision(3)
             << (double) str_to_int(read_file("/sys/class/thermal/thermal_zone" + to_string(i) + "/" + "temp")) / 1000
             << " Degree Celsius"
             << endl;
    }
}

void
get_cpu_usage_raw(unsigned long long &total_user, unsigned long long &total_user_low, unsigned long long &total_sys,
                  unsigned long long &total_idle) {
    FILE *file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &total_user, &total_user_low, &total_sys, &total_idle);
    fclose(file);
}

void show_cpu_usage(const unsigned long long &last_total_user, const unsigned long long &last_total_user_low,
                    const unsigned long long &last_total_sys, const unsigned long long &last_total_idle) {
    unsigned long long total_user, total_user_low, total_sys, total_idle;
    get_cpu_usage_raw(total_user, total_user_low, total_sys, total_idle);

    double total = (double) (total_user - last_total_user) + (double) (total_user_low - last_total_user_low) +
                   (double) (total_sys - last_total_sys);
    total = total * 100 / (total + (double) (total_idle - last_total_idle));
    cout << " - Usage: " << fixed << setprecision(2) << total << '%';
    total = (double) total_user + (double) total_user_low + (double) total_sys;
    total = total * 100 / (total + (double) total_idle);
    cout << endl
         << " - Overall Usage: " << fixed << setprecision(2) << total << '%' << endl;
}

void show_memory_info() {
    // Get memory data with linux command 'free'
    FILE *cmd = popen("free", "r");
    // Ignore first line
    char line[1024];
    try {
        fgets(line, 1024, cmd);
    } catch (error_t) {}
    fgets(line, 1024, cmd);
    show_physical_memory_info(line);
    fgets(line, 1024, cmd);
    show_virtual_memory_info(line);
}

void show_physical_memory_info(const char line[1024]) {

    char str_title[32];
    int mem_total;
    int mem_used;
    int mem_free;
    int mem_shared;
    int mem_buffered;
    int mem_available;
    sscanf(line, "%s%d%d%d%d%d%d", str_title, &mem_total, &mem_used, &mem_free, &mem_shared, &mem_buffered, &mem_available);
    cout << " - Physical RAM: " << fixed << setprecision(0) << (double) mem_used * 100 / (double) mem_total << "%, "
         << kilobyte_to_megabyte(mem_used) << " MiB out of "
         << kilobyte_to_megabyte(mem_total) << " MiB used." << endl;
}

void show_virtual_memory_info(const char line[1024]) {
    char str_title[32];
    int mem_total;
    int mem_used;
    int mem_free;
    int mem_shared;
    int mem_buffered;
    int mem_available;
    sscanf(line, "%s%d%d%d%d%d%d", str_title, &mem_total, &mem_used, &mem_free, &mem_shared, &mem_buffered, &mem_available);

    cout << " - Swap: " << fixed << setprecision(0) << (double) mem_used * 100 / (double) mem_total << "%, "
         << kilobyte_to_megabyte(mem_used) << " MiB out of "
         << kilobyte_to_megabyte(mem_total) << " MiB used." << endl;
}

int is_dir_exists(const string &path) {
    return is_dir_exists(path.c_str());
}

int is_dir_exists(const char *path) {
    struct stat info{};
    return !stat(path, &info) && info.st_mode & S_IFDIR;
}

int is_file_exists(const string &path) {
    return is_file_exists(path.c_str());
}

int is_file_exists(const char *path) {
    struct stat info{};
    return !stat(path, &info) && info.st_mode & S_IFREG;
}

string read_file(const string &path) {
    ifstream ifstream;
    ifstream.open(path);
    stringstream string_stream;
    string_stream << ifstream.rdbuf();
    ifstream.close();
    string string = string_stream.str();
    if (string.at(string.length() - 1) == '\n') {
        string = string.substr(0, string.length() - 1);
    }
    return string;
}

int str_to_int(const string &str) {
    int tmp = 0;
    for (char char_str : str) {
        tmp = (tmp * 10) + char_str - 48;
    }
    return tmp;
}

long kilobyte_to_megabyte(const long &kilobyte) {
    return kilobyte / 1024;
}