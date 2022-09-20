#include "debug.h"

long first_time = 0;

long getTimeMillesec() {
    // https://stackoverflow.com/questions/19555121/how-to-get-current-timestamp-in-milliseconds-since-1970-just-the-way-java-gets

    if(!first_time) {
        first_time = 1;
        first_time = getTimeMillesec();
    }

    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - first_time;
}
