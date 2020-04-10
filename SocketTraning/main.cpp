#include <iostream>
//#include <sys/types.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <string.h>
//#include <string>
#include "TensorFlowSocket.h"

using namespace std;

int main()
{
    TensorFlowSocket s;

    while (1)
    {
        cout << "> ";
        string userInput;
        getline(cin, userInput);
        string output = s.runModel(userInput);

        cout << "SERVER> " << output << endl;

    }
    return 0;
}