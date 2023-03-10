#include<cstdio>
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<cstdlib>
#include <unistd.h>
#include<string>
#include<random>
#include<utility>
#include<chrono>

using namespace std;

// Main function to measure elapsed time of a C++ program
// using Chrono library
int main()
{
    auto start = chrono::steady_clock::now();

    // do some stuff here
    sleep(3);


    auto end = chrono::steady_clock::now();

    cout << "Elapsed time in nanoseconds: "
        << chrono::duration_cast<chrono::nanoseconds>(end - start).count()
        << " ns" << endl;

    cout << "Elapsed time in microseconds: "
        << chrono::duration_cast<chrono::microseconds>(end - start).count()
        << " ?s" << endl;

    cout << "Elapsed time in milliseconds: "
        << chrono::duration_cast<chrono::milliseconds>(end - start).count()
        << " ms" << endl;

    cout << "Elapsed time in seconds: "
        << chrono::duration_cast<chrono::seconds>(end - start).count()
        << " sec";

    return 0;
}
