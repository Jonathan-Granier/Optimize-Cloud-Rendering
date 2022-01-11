#include "Prime.h"
#include <cmath>

//----------------------------------------------------------------------------------------------------------------------
std::vector<int> Sieve()
{
    constexpr int n = 100'000'000; //Maximum
    std::vector<int> primes;
    // In general Sieve of Sundaram, produces primes
    // smaller than (2*x + 2) for a number given
    // number x
    int nNew = static_cast<int>(std::sqrt(n));

    // This array is used to separate numbers of the
    // form i+j+2ij from others where  1 <= i <= j
    std::vector<int> marked(n / 2 + 500, 0);

    // eliminate indexes which does not produce primes
    for (int i = 1; i <= (nNew - 1) / 2; i++)
        for (int j = (i * (i + 1)) << 1; j <= n / 2; j = j + 2 * i + 1)
            marked[j] = 1;

    // Since 2 is a prime number
    primes.push_back(2);

    // Remaining primes are of the form 2*i + 1 such
    // that marked[i] is false.
    for (int i = 1; i <= n / 2; i++)
        if (marked[i] == 0)
            primes.push_back(2 * i + 1);
    return primes;
}

//----------------------------------------------------------------------------------------------------------------------
int binarySearch(std::vector<int> &primes, int left, int right, int n)
{
    if (left <= right)
    {
        int mid = (left + right) / 2;

        // base condition is, if we are reaching at left
        // corner or right corner of primes[] array then
        // return that corner element because before or
        // after that we don't have any prime number in
        // primes array
        if (mid == 0 || mid == static_cast<int32_t>(primes.size()) - 1)
            return primes[mid];

        // now if n is itself a prime so it will be present
        // in primes array and here we have to find nearest
        // prime less than n so we will return primes[mid-1]
        if (primes[mid] == n)
            return primes[mid - 1];

        // now if primes[mid]<n and primes[mid+1]>n that
        // mean we reached at nearest prime
        if (primes[mid] < n && primes[mid + 1] > n)
            return primes[mid];
        if (n < primes[mid])
            return binarySearch(primes, left, mid - 1, n);
        else
            return binarySearch(primes, mid + 1, right, n);
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
int FindPreviousClosestPrime(uint32_t N)
{
    std::vector<int> primes = Sieve();
    uint32_t n = binarySearch(primes, 0, static_cast<uint32_t>(primes.size()) - 1, N);
    while (n % 4 != 3)
    {
        n = binarySearch(primes, 0, static_cast<uint32_t>(primes.size()) - 1, n - 1);
    }
    return n;
}