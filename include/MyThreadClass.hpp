/*
 * MyThreadClass.hpp
 *
 *  Created on: 05.02.2015
 *      Author: dnoack
 */

#ifndef INCLUDE_MYTHREADCLASS_HPP_
#define INCLUDE_MYTHREADCLASS_HPP_



#include <pthread.h>
#include <stdlib.h>

class MyThreadClass;

struct argStruct{
	MyThreadClass* th_id;
	int socket;
};


/**
\class MyThreadClass
\brief Implements Thread-handling for two threads within an object.

MyThreadClass defines two methods thread_work() and thread_listen(prthread_t) which have to be implemented by
classes who inherit from MyThreadClass. The class got two internal variables for two threads. The class is intended to work like this:
- thread_work() have to do all the connection setup and executing commands if something is received from a client, the function will run in thread _worker.
- thread_lisen() got a endless loop, where it waits for receiving data from a client, the cuntion will run in thread _listener.

- If thread_listen receives some data (within the thread _listener) it will signal the thread _worker.
- The thread _worker will check the signal within the function thread_worker() and do some computation or whatever.
- As long as the worker is busy, the listener can receive commands, but they will not be executed.
- After finishing a computation, the worker will get ready again and can handler further commands from the listener.
*/
class MyThreadClass
{
public:

	/**Constructor*/
   MyThreadClass()
   {
	   _worker = 0;
	   _listener = 0;
	   _accepter = 0;
   }
   /**Destructor*/
   virtual ~MyThreadClass() {/* empty */}

   /**
   Starts the worker thread, which start executing the thread_work() function.
   \return True if the thread was successfully started, false if there was an error starting the thread.*/
   pthread_t StartWorkerThread(int socket)
   {
	  tempStruct.socket = socket;
	  tempStruct.th_id = this;

	  pthread_create(&_worker, NULL, thread_workEntryFunc, &tempStruct);
	  return _worker;
   }

   /**
   Starts the listen thread, which start executing the thread_listen() function.
   \return True if the thread was successfully started, false if there was an error starting the thread.*/
   pthread_t StartListenerThread(pthread_t parent_th, int socket)
   {
	   tempStruct.socket = socket;
	   tempStruct.th_id = this;

	   pthread_create(&_listener, NULL, thread_listenerEntryFunc, &tempStruct);
	   return _listener;
   }


   int StartAcceptThread()
   {
	   return(pthread_create(&_accepter, NULL, thread_acceptEntryFunc, this) == 0);
   }


   /** Will not return until the internal worker thread has exited. */
   void WaitForWorkerThreadToExit()
   {
      (void) pthread_join(_worker, NULL);
   }

   /** Will not return until the internal listener thread has exited. */
   void WaitForListenerThreadToExit()
   {
	   (void) pthread_join(_listener, NULL);
   }

   void WaitForAcceptThreadToExit()
   {
	   (void) pthread_join(_accepter, NULL);
   }

protected:

	/** This method has to be responsible for the connection setup and execution tasks like prngd computation.*/
   virtual void thread_work(int) = 0;
   /**This method has to be responsible for listening to incomming data and signaling the worker.*/
   virtual void thread_listen(pthread_t, int)= 0;

   virtual void thread_accept()= 0;

private:
	/** Creates a new worker thread and starts executing thread_work within it.*/
   static void* thread_workEntryFunc(void * This)
   {
	   MyThreadClass* mtc = ((argStruct*)This)->th_id;
	   int socket =  ((argStruct*)This)->socket;

	   mtc->thread_work(socket);
	   return NULL;
   }
   /** Creates a new listener thread and starts executing thread_listen within it.*/
   static void* thread_listenerEntryFunc(void* This)
   {
	   MyThreadClass* mtc = ((argStruct*)This)->th_id;
	   int socket =  ((argStruct*)This)->socket;

	   mtc->thread_listen(mtc->_worker, socket);
	   //((MyThreadClass*)This)->thread_listen(((MyThreadClass*)This)->_worker, socket);
	   return NULL;
   }

   static void* thread_acceptEntryFunc(void* This) {((MyThreadClass*)This)->thread_accept(); return NULL;}

   /** Id of the internal worker thread.*/
   pthread_t _worker;
   /** ID of the internal listener thread.*/
   pthread_t _listener;

   pthread_t _accepter;

   argStruct tempStruct;
};




#endif /* INCLUDE_MYTHREADCLASS_HPP_ */
