/**
  communication-queue.h - Created by Timothy Morey on 3/16/2012.

  This file defines an interface by which threads may communicate.  A queue
  is defined that can be managed by one thread and can accept messages/requests
  from many other threads.
*/

#ifndef __COMMUNICATION_QUEUE_H
#define __COMMUNICATION_QUEUE_H

#define ARGS_SIZE 8
#define MSG_SIZE 32
#define QUEUE_SIZE 32

#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  char Message[MSG_SIZE];
  void* Arguments[ARGS_SIZE];
  int DestroyArgs;
  int* Handled;
  void* Result;
  size_t ResultSize;
} Message;

typedef struct
{
  Message Messages[QUEUE_SIZE];
  int Capacity;
  int Size;
  pthread_mutex_t Mutex;
} CommunicationQueue;

/**
  @return An nvn error code indicating if the operation was successful.
*/
int DestroyArguments(Message msg);

/**
  @return An nvn error code indicating if the operation was successful.
*/
int DestroyQueue(CommunicationQueue queue);

/**
  @return An nvn error code indicating if the operation was successful.
*/
int InitMessage(Message* msg, const char* text);

/**
  @return An nvn error code indicating if the operation was successful.
*/
int InitQueue(CommunicationQueue* queue);

/**
  @param[in] queue The queue from which we will pop a message.
  @param[out] msg The message that was popped.
  @param[out] valid If the queue was empty, then we couldn't pop a message and
  this int will be set to 0.  If we successfully popped a message, this will be
  set to 1.

  @return An nvn error code indicating if the operation was successful.
*/
int Pop(CommunicationQueue* queue, Message* msg, int* valid);

/**
  @return An nvn error code indicating if the operation was successful.
*/
int Push(CommunicationQueue* queue, Message msg);

#ifdef __cplusplus
}
#endif

#endif
