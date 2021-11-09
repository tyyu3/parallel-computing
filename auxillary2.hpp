#ifndef AUXILLARY2_HPP
#define AUXILLARY2_HPP

#include <iostream>

namespace aux
{
    struct Timings
    {
        std::size_t iterations;
        std::uint64_t ticks;
        std::uint64_t ns;
    };
}

#endif // AUXILLARY2_HPP
