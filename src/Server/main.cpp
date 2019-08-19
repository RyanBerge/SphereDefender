#include <iostream>
#include "server.h"

using std::cout, std::endl;

int main()
{
    Server server;

    cout << "Launching server..." << endl;

    while (true)
    {
        server.Update();
    }
}
