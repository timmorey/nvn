/**
  communication-queue.c - Created by Timothy Morey on 3/16/2012.

  This file implements the interface defined in communication-queue.h.
*/

#include <stdlib.h>
#include <string.h>

#include "communication-queue.h"
#include "nvn.h"


/*****************************************************************************
 * communication-queue.h implementations:
 *****************************************************************************/

int DestroyArguments(Message msg)
{
  int retval = NVN_NOERR;
  int i = 0;

  if(msg.DestroyArgs)
  {
    for(i = 0; i < ARGS_SIZE; i++)
    {
      if(msg.Arguments[i])
        free(msg.Arguments[i]);
    }
  }

  return retval;
}

int DestroyQueue(CommunicationQueue queue)
{
  int retval = NVN_NOERR;

	retval = 0 == pthread_mutex_destroy(&queue.Mutex);

  return retval;
}

int InitMessage(Message* msg, const char* text)
{
  int retval = NVN_NOERR;

  if(msg && text)
  {
    memset(msg, 0, sizeof(Message));
    strcpy(msg->Message, text);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int InitQueue(CommunicationQueue* queue)
{
  int retval = NVN_NOERR;

  if(queue)
  {
    int i = 0;

    queue->Capacity = QUEUE_SIZE;
    queue->Size = 0;
    for(i = 0; i < QUEUE_SIZE; i++)
      InitMessage(&queue->Messages[i], "");
		pthread_mutex_init(&queue->Mutex, 0);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int Pop(CommunicationQueue* queue, Message* msg, int* valid)
{
  int retval = NVN_NOERR;

  if(queue && msg)
  {
	  InitMessage(msg, "INVALID");
    if(valid)
      *valid = 0;

    if(queue->Size > 0)
    {
      int i = 0;
    
			pthread_mutex_lock(&queue->Mutex);

		  if(queue->Size > 0)
		  {
			  *msg = queue->Messages[0];
        if(valid)
          *valid = 1;

			  for(i = 1; i < queue->Size; i++)
				  queue->Messages[i-1] = queue->Messages[i];
			  queue->Size--;
		  }

			pthread_mutex_unlock(&queue->Mutex);
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int Push(CommunicationQueue* queue, Message msg)
{
  int retval = NVN_NOERR;

  if(queue)
  {
    int pushed = 0;
    
    while(!pushed)
    {
      while(queue->Size >= queue->Capacity)
        pthread_yield();

			pthread_mutex_lock(&queue->Mutex);

		  if(queue->Size < queue->Capacity)
		  {
			  queue->Messages[queue->Size] = msg;
        queue->Size++;
        pushed = 1;
		  }

			pthread_mutex_unlock(&queue->Mutex);
    }
  }

  return retval;
}
