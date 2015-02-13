/*
 * UdsWorker.hpp
 *
 *  Created on: 09.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_UDSWORKER_HPP_
#define INCLUDE_UDSWORKER_HPP_

//unix domain socket definition
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <list>


#include "JsonRPC.hpp"
#include "Aardvark.hpp"
#include "PluginAardvark.hpp"
#include <pthread.h>
#include "WorkerThreads.hpp"
#include "signal.h"

class UdsServer;


#define BUFFER_SIZE 1024
#define ADD_WORKER true
#define DELETE_WORKER false
#define WORKER_FREE 0
#define WORKER_BUSY 1
#define WORKER_GETSTATUS 2


class UdsWorker : public WorkerThreads{

	public:
		UdsWorker(int socket);
		~UdsWorker();


	private:


		//variables for listener
		bool listen_thread_active;
		char receiveBuffer[BUFFER_SIZE];
		int recvSize;


		//variables for worker
		bool worker_thread_active;
		PluginAardvark* paard;
		string* request;
		string* response;


		//shared variables
		pthread_mutex_t wBusyMutex;
		bool workerBusy;
		//receivequeue
		list<string*> receiveQueue;
		pthread_mutex_t rQmutex;


		//not shared, more common
		pthread_t lthread;
		int optionflag;
		struct sockaddr_un address;
		int currentSocket;

		socklen_t addrlen;
		sigset_t sigmask;
		struct sigaction action;
		struct sigaction pipehandler;
		int currentSig;


		virtual void thread_listen(pthread_t partent_th, int socket, char* workerBuffer);

		virtual void thread_work(int socket);

		static void dummy_handler(int){};

		bool workerStatus(int status);

		//data + add = new msg , data = NULL + add=false = remove oldest
		void editReceiveQueue(string* data, bool add)
		{
			pthread_mutex_lock(&rQmutex);
			if(data != NULL)
			{

				if(add)
				{
					receiveQueue.push_front(data);
				}
			}
			else
			{
				if(!add)
				{
					receiveQueue.pop_back();
				}
			}
			pthread_mutex_unlock(&rQmutex);
		}



		void configSignals()
		{
			sigfillset(&sigmask);
			pthread_sigmask(SIG_UNBLOCK, &sigmask, (sigset_t*)0);

			action.sa_flags = 0;
			action.sa_handler = dummy_handler;
			sigaction(SIGUSR1, &action, (struct sigaction*)0);
			sigaction(SIGUSR2, &action, (struct sigaction*)0);
			sigaction(SIGPOLL, &action, (struct sigaction*)0);
			sigaction(SIGPIPE, &action, (struct sigaction*)0);
		}




};



#endif /* INCLUDE_UDSWORKER_HPP_ */
