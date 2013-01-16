/**
  communication-queue.c - Created by Timothy Morey on 3/16/2012.

  This file implements the interface defined in communication-queue.h.
*/

#include <stdio.h>
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

int EqualMessagesP(Message m1, Message m2)
{
  int equal = 0;
  int i;

  equal = 0 == strcmp(m1.Message, m2.Message);
  
  for(i = 0; equal && i < ARGS_SIZE; i++)
    equal = m1.Arguments[i] == m2.Arguments[i];

  return equal;
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
    queue->Front = 0;
    queue->Size = 0;
    for(i = 0; i < QUEUE_SIZE; i++)
      InitMessage(&queue->Messages[i], "INVALID");
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
			  *msg = queue->Messages[queue->Front];
        if(valid)
          *valid = 1;

        queue->Size--;
        queue->Front = (queue->Front + 1) % queue->Capacity;
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

    if(queue->Size < queue->Capacity)
    {
      pthread_mutex_lock(&queue->Mutex);

      if(queue->Size < queue->Capacity)
      {
        queue->Messages[(queue->Front + queue->Size) % queue->Capacity] = msg;
        queue->Size++;
        pushed = 1;
      }

			pthread_mutex_unlock(&queue->Mutex);
    }
    
    if(! pushed)
    {
      retval = NVN_EQFULL;
      fprintf(stderr, "Queue full - '%s' message dropped.\n", msg.Message);
    }
  }

  return retval;
}

int PushIfUnique(CommunicationQueue* queue, Message msg)
{
  int retval = NVN_NOERR;

  if(queue)
  {
    int full = 0;
    int found = 0;

    if(QueueContainsP(queue, msg))
    {
      found = 1;
    }
    else if (queue->Size >= queue->Capacity)
    {
      full = 1;
    }
    else
    {
      pthread_mutex_lock(&queue->Mutex);

      if(QueueContainsP(queue, msg))
      {
        found = 1;
      }
      else if (queue->Size >= queue->Capacity)
      {
        full = 1;
      }
      else
      {
        queue->Messages[(queue->Front + queue->Size) % queue->Capacity] = msg;
        queue->Size++;
      }

			pthread_mutex_unlock(&queue->Mutex);
    }
    
    if(found)
    {
      retval = NVN_ENOTUNIQUE;
      printf("Dropped non-unique message.\n");
    }
    else if(full)
    {
      retval = NVN_EQFULL;
      fprintf(stderr, "Queue full - '%s' message dropped.\n", msg.Message);
    }
  }

  return retval;
}

int QueueContainsP(CommunicationQueue* queue, Message msg)
{
  int contains = 0;
  int i;

  if(queue)
  {
    for(i = queue->Front; i < queue->Front + queue->Size; i++)
    {
      if(EqualMessagesP(msg, queue->Messages[i % queue->Capacity]))
      {
        contains = 1;
        break;
      }
    }
  }

  return contains;
}

