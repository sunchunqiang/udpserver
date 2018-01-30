/*
 * udpserver.h
 *
 *  Created on: Jan 27, 2018
 *      Author: csun
 */

#ifndef UDPSERVER_H_
#define UDPSERVER_H_
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include  <cstring>
#include <string>
#include "threadpool.h"
using namespace std;





class udpserver {
	int port;
	int bufflen;
	bool done = false;
	void log(const string& msg)
	{
		cout << msg << endl;
	}
	thread_pool workingthreads;
public:
	udpserver(int _port, int _bufflen = 1024):port(_port), bufflen(_bufflen)
	{

	}
	virtual ~udpserver()
	{

	}
	int run();
};

#endif /* UDPSERVER_H_ */
