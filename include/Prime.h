#pragma once
#include <cstdint>
#include <vector>
// Utility function of Sieve of Sundaram
std::vector<int> Sieve();

// modified binary search to find nearest prime less than N
int binarySearch(std::vector<int> &primes, int left, int right, int n);

int FindPreviousClosestPrime(uint32_t N);

// From https://preshing.com/20121224/how-to-generate-a-sequence-of-unique-random-integers/
[[maybe_unused]] static uint32_t Permute(uint32_t prime, uint32_t x)
{
    if (x >= prime)
        return x; // The 5 integers out of range are mapped to themselves.
    uint32_t residue = static_cast<uint32_t>((static_cast<unsigned long long>(x) * x) % prime);
    return (x <= prime / 2) ? residue : prime - residue;
}