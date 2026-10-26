#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "server.hpp"
using namespace std;

int main()
{
    Server server(9999);
    server.start();
    return 0;
}