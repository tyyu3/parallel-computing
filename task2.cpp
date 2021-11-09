#include "auxillary2.hpp"

#include <array>
#include <chrono>
#include <random>
#include <vector>

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

void blur_basic(const std::vector<std::array<std::uint8_t,3>>& orig, std::vector<std::array<std::uint8_t,3>>& blur)
{
    asm volatile("# blur_basic start");
    for(size_t i = 1; i < orig.size()-1; ++i)
    {
        blur[i][0] = orig[i-1][0]/2 + orig[i+1][0]/2;
        blur[i][1] = orig[i-1][1]/2 + orig[i+1][1]/2;
        blur[i][2] = orig[i-1][1]/2 + orig[i+1][2]/2;
    }
    asm volatile("# blur_basic end");

}
void blur_simd(const std::vector<std::array<std::uint8_t,3>>&  orig, std::vector<std::array<std::uint8_t,3>>& blur)
{
    asm volatile("# blur_simd start");
    #pragma omp simd
    for(size_t i = 1; i < orig.size()-1; ++i)
    {
        blur[i][0] = orig[i-1][0]/2 + orig[i+1][0]/2;
        blur[i][1] = orig[i-1][1]/2 + orig[i+1][1]/2;
        blur[i][2] = orig[i-1][1]/2 + orig[i+1][2]/2;
    }
    asm volatile("# blur_simd end");
}

void blur_parallel(const std::vector<std::array<std::uint8_t,3>>&  orig, std::vector<std::array<std::uint8_t,3>>& blur)
{
    asm volatile("# blur_parallel start");
    #pragma omp parallel for num_threads(8)
    for(size_t i = 1; i < orig.size()-1; ++i)
    {
        blur[i][0] = orig[i-1][0]/2 + orig[i+1][0]/2;
        blur[i][1] = orig[i-1][1]/2 + orig[i+1][1]/2;
        blur[i][2] = orig[i-1][1]/2 + orig[i+1][2]/2;
    }
    asm volatile("# blur_parallel end");
}

template<typename Pixels, typename Func>
aux::Timings mesure_blur(const Func& func, const Pixels& pic_old, Pixels& pic_new, std::size_t iter)
{
    pic_new.resize(pic_old.size());

    auto time_start = std::chrono::high_resolution_clock::now();
    std::uint64_t ticks_start = ticks();
    for (std::size_t i = 0; i < iter; ++i)
    {
        func(pic_old, pic_new);
        do_not_optimize(pic_new);
    }
    auto time_end = std::chrono::high_resolution_clock::now();
    std::uint64_t ticks_end = ticks();

    aux::Timings result {
            .iterations = iter,
            .ticks = (ticks_end - ticks_start),
            .ns = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count())
                        };
    return result;
}
int main()
{
    std::default_random_engine eng(1);
    std::vector<std::array<std::uint8_t,3>> pic_original, pic_blur;
    std::mt19937 rng(eng());
    std::uniform_int_distribution<std::mt19937::result_type> dist255(0,255);

    size_t height = 3000000;
    pic_original.resize(height);

    for(int i = 0; i < height; ++i)
    {
        pic_original[i] = {static_cast<std::uint8_t>(dist255(rng)),
                                static_cast<std::uint8_t>(dist255(rng)),
                                static_cast<std::uint8_t>(dist255(rng))};

    }

    std::cout << "iter,ms,ticks" << '\n';
    for (std::uint64_t iter = 100;  iter <= 100000; iter*=1000)
    {
        aux::Timings result;
        result = mesure_blur(blur_basic, pic_original, pic_blur, iter);
        std::cout << result.iterations << ',' << result.ns/1000000 << ',' << result.ticks << '\n';
        result = mesure_blur(blur_simd, pic_original, pic_blur, iter);
        std::cout << result.iterations << ',' << result.ns/1000000 << ',' << result.ticks << '\n';
        result = mesure_blur(blur_parallel, pic_original, pic_blur, iter);
        std::cout << result.iterations << ',' << result.ns/1000000 << ',' << result.ticks << '\n';
    }

    return 0;
}

