#include "Driver.h"

#include <iostream>
#include <stdexcept>

int main() 
{
    Driver d;

    try
    {
        while(d.loopFlag) d.Loop();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}