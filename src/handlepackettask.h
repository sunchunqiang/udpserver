/*
 * handlepackettask.h
 *
 *  Created on: Jan 27, 2018
 *      Author: csun
 */

#ifndef HANDLEPACKETTASK_H_
#define HANDLEPACKETTASK_H_

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <thread>
#include <sstream>
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
using namespace std;
using namespace rapidjson;

class handlepackettask
{
	struct Packet
	{
		string message;
		sockaddr_in  si_client;
		int server_socket ;
		Packet(const string& message_, const sockaddr_in& _clinet, int _socket )
		: message(message_), si_client(_clinet), server_socket(_socket)
		{

		}
	};

	unique_ptr<Packet>  package;


public:
	handlepackettask(const string& message, const sockaddr_in& clinet , int _socket )
		: package(new Packet(message,clinet,_socket))
	{

	}
	handlepackettask(handlepackettask&& other ): package(move(other.package))
	{

	}
	int operator()()
	{

		stringstream ss;
		ss << this_thread::get_id();

		Document document;  // Default template parameter uses UTF8





			int index = 0 ;
			string responsemsg ;

			if (document.Parse(package->message.c_str()).HasParseError())
			{
				responsemsg = "failed to parse the request message";
			}
			else
			{
				index = document["index"].GetInt();
				responsemsg ="successfully parsed the request message";

			}
		sockaddr_in  si_client = package-> si_client;

		StringBuffer s;
			Writer<StringBuffer> writer(s);
			writer.StartObject();                // Between StartObject()/EndObject(),
			writer.Key("response");                // output a key,[=l
			writer.String(responsemsg.c_str());             // follow by a value.

			writer.Key("index");
			writer.Uint(index);
			writer.EndObject();

			const char* msg  = s.GetString();



		sendto(package->server_socket, msg, strlen(msg),0, (sockaddr*)&si_client, sizeof(si_client));


//		cout << this_thread::get_id() << ": handle message" << package->message << endl;
//	   printf("Received packet from %s:%d\nData\n\n",
//					inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port));
		return package->message.size();
	}
};




#endif /* HANDLEPACKETTASK_H_ */
