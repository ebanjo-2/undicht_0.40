#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <chrono>

// macros for logging and debugging

#define UND_ERROR std::cout << "ERROR: " <<  " from " << __FILE__ << " : " << __LINE__ << "\n    "

#define UND_WARNING std::cout << "WARNING: " << " from " << __FILE__ << "\n    "

#define UND_LOG std::cout << std::flush << "Note: " <<  getTimeMillesec() << " ms\n    "

long getTimeMillesec();


#endif // DEBUG_H
