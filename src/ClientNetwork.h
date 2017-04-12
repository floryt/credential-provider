#pragma once
// Networking libraries
#include <winsock2.h>
#include <Windows.h>
#include "NetworkServices.h"
#include <ws2tcpip.h>
#include <stdio.h>
#include "NetworkData.h"

// size of our buffer
#define DEFAULT_BUFLEN 512
// port to connect sockets through
#define DEFAULT_PORT "8989"
#define DEFAULT_IP "192.168.2.112"
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

/*
The base was copied from the internet, the rest was wrriten by BAR (for now)
*/
class ClientNetwork
{
public:

	// for error checking function calls in Winsock library
	int iResult;

	// socket for client to connect to server
	SOCKET ConnectSocket;

	// ctor/dtor
	ClientNetwork(void);
	~ClientNetwork(void);

	int receivePackets(char *);
	int sendPackets(char *);
	bool getPremmission(); //the foo that recived True or False from the service
};
