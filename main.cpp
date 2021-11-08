#include "auxillary.hpp"

#include <immintrin.h>

#include <iostream>
#include <chrono>
#include <set>

static constexpr std::size_t iterations = 1000*1000000;
static constexpr std::size_t vector_size = sizeof(__m256i)/sizeof(std::int64_t);
static constexpr std::size_t vector_iterations = iterations / vector_size;

template <typename T>
inline __attribute__((always_inline)) void do_not_optimize(T& value)
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

inline __attribute__((always_inline)) std::uint64_t ticks()
{
    std::uint64_t tsc;
    asm volatile("mfence; "         // memory barrier
                 "rdtsc; "          // read of tsc
                 "shl $32,%%rdx; "  // shift higher 32 bits stored in rdx up
                 "or %%rdx,%%rax"   // and or onto rax
                 : "=a"(tsc)        // output to tsc
                 :
                 : "%rcx", "%rdx", "memory");
    return tsc;
}

aux::Timings scalar_independent_abs(std::int64_t x)
{
    std::int64_t y;
    asm volatile("# scalar_independent_abs start");
    auto time_start = std::chrono::high_resolution_clock::now();
    auto ticks_start = ticks();
    for (std::size_t i = 0; i < iterations; ++i)
    {
        y = std::abs(x);
        do_not_optimize(y);
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    auto ticks_end = ticks();
    asm volatile("# scalar_independent_abs end");
    aux::Timings result {
            .iterations = iterations,
            .ticks_per_iter = static_cast<double>(ticks_end - ticks_start)/iterations,
            .ns_per_iter = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count())/iterations
                        };
    return result;
};

aux::Timings scalar_dependent_abs(std::int64_t x)
{
    asm volatile("# scalar_dependent_abs start");
    auto time_start = std::chrono::high_resolution_clock::now();
    auto ticks_start = ticks();
    for (std::size_t i = 0; i < iterations; ++i)
    {
        x = std::abs(x);
        do_not_optimize(x);
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    auto ticks_end = ticks();
    asm volatile("# scalar_dependent_abs end");
    aux::Timings result {
            .iterations = iterations,
            .ticks_per_iter = static_cast<double>(ticks_end - ticks_start)/iterations,
            .ns_per_iter = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count())/iterations
                        };
    return result;
};

void put_scalar_into_vector(std::int64_t scalar, __m256i& vector)
{
    std::int64_t array[vector_size];
    for (std::size_t i = 0; i < vector_size; ++i)
        array[i] = scalar;
    std::memcpy(&vector, array, sizeof(vector));
}

aux::Timings vector_independent_abs(std::int64_t x)
{
    __m256i v1, v2;
    put_scalar_into_vector(x, v1);
    asm volatile("# vector_independent_abs start");
    auto time_start = std::chrono::high_resolution_clock::now();
    auto ticks_start = ticks();
    for (std::size_t i = 0; i < vector_iterations; ++i)
    {
        v2 = _mm256_abs_epi32(v1);
        do_not_optimize(v2);
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    auto ticks_end = ticks();
    asm volatile("# vector_independent_abs end");
    aux::Timings result {
            .iterations = iterations,
            .ticks_per_iter = static_cast<double>(ticks_end - ticks_start)/iterations,
            .ns_per_iter = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count())/iterations
                        };
    return result;
}

aux::Timings vector_dependent_abs(std::int64_t x)
{
    __m256i v1;
    put_scalar_into_vector(x, v1);

    asm volatile("# vector_dependent_abs start");
    auto time_start = std::chrono::high_resolution_clock::now();
    auto ticks_start = ticks();
    for (std::size_t i = 0; i < vector_iterations; ++i)
    {
        v1 = _mm256_abs_epi32(v1);
        do_not_optimize(v1);
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    auto ticks_end = ticks();
    asm volatile("# vector_dependent_abs end");
    aux::Timings result {
            .iterations = iterations,
            .ticks_per_iter = static_cast<double>(ticks_end - ticks_start)/iterations,
            .ns_per_iter = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count())/iterations
                        };
    return result;
}

int main(int argc, char* argv[])
{
    auto parsed = aux::parse_cmd(argc, argv);
    if(parsed.second != true)
        return 1;
    std::int64_t x = parsed.first;
    auto result = scalar_independent_abs(x);
    std::cout << result.iterations << ' ' << result.ns_per_iter << ' ' << result.ticks_per_iter << '\n';
    result = scalar_dependent_abs(x);
    std::cout << result.iterations << ' ' << result.ns_per_iter << ' ' << result.ticks_per_iter << '\n';
    result = vector_independent_abs(x);
    std::cout << result.iterations << ' ' << result.ns_per_iter << ' ' << result.ticks_per_iter << '\n';
    result = vector_dependent_abs(x);
    std::cout << result.iterations << ' ' << result.ns_per_iter << ' ' << result.ticks_per_iter << '\n';
    return 0;
}
