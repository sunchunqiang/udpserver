//============================================================================
// Name        : udpserver.cpp
// Author      : csun
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <string>
#include <atomic>
#include <thread>
#include "udpserver.h"
#include "handlepackettask.h"

using namespace std;
using namespace std::chrono;

void test_queue();


int main() {
	udpserver server(9020);
	server.run();
	return 0;
}


int udpserver::run() {
			sockaddr_in si_server , si_client;
			int s ;
			char buf[bufflen];

			cout <<"start " << endl;

			if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
			{
				log("socket");
			}


			memset((char *) &si_server, 0, sizeof(si_server));
			si_server.sin_family = AF_INET;
			si_server.sin_port = htons(port);
			si_server.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(s, (sockaddr *)&si_server, sizeof(si_server)) == -1)
			{
				log("bind");
			}

			socklen_t slen =  sizeof(si_client);
			workingthreads.start(4);
			done = false;
			cout << " start service ..." << endl;
			while(!done)
			{
				int len = recvfrom(s, buf, bufflen, 0, (sockaddr *)&si_client, &slen);
				if ( len == -1)
				{
					log("recvfrom() error");
					return -1;

				}

				buf[len] = 0;

				handlepackettask task(buf, si_client,s);
				workingthreads.submit(task);
 			    printf("Received packet from %s:%d\nData: %s\n\n",
  						inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port), buf,len);

			}

			close(s);
			return 0;
		}

