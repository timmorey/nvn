/**
  server.c - Created by Timothy Morey on 4/5/2012.
*/


#include "nvn.h"

#include "server.h"
#include "socket-utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WINDOWS__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#else
#include <netinet/in.h>
#include <pthread.h>
#endif

#ifdef __WINDOWS__
#pragma warning(disable:4996)
#endif


/*****************************************************************************
 * Local definitions:
 *****************************************************************************/

typedef struct
{
  Server Base;
  int ServerThreadAlive;
  int KeepServerAlive;
  SOCKET Socket;
#ifdef __WINDOWS__
  HANDLE ThreadHandle;     // The windows handle to the thread.
  DWORD ThreadId;          // The id of the thread.
#else
  pthread_t ThreadHandle;  // The pthread thread handle.
#endif
} ServerEx;

int CheckReceiveBuffer(ServerEx* server, int capacity);
int CheckSendBuffer(ServerEx* server, int capacity);
int FreeServerResources(ServerEx* server);
int HandleChattyClient(ServerEx* server, SOCKET acceptSocket, 
                       const char* clientName);
int HandleStreamClient(ServerEx* server, SOCKET acceptSocket, 
                       const char* clientName);
int InvokeCallback(ServerEx* server, const char* clientName, int msgSize, int* responseSize);
int ServerLoop(ServerEx* server);

#ifdef __WINDOWS__
DWORD WINAPI ServerThreadEntryPoint(LPVOID arg);
#else
void* ServerThreadEntryPoint(void* arg);
#endif


/*****************************************************************************
 * Implementation of server.h functions:
 *****************************************************************************/

int StartAsyncServer(int port, ServerRequestCallback callback, ServerMode mode, Server** server)
{
  int retval = NVN_NOERR;

  if(server)
  {
    ServerEx* sex = (ServerEx*)malloc(sizeof(ServerEx));
    struct sockaddr_in serverAddr;

    memset(sex, 0, sizeof(ServerEx));
    sex->Base.Port = port;
    sex->Base.RequestCallback = callback;
    sex->Base.Async = 1;
    sex->Base.Mode = mode;

    sex->Socket = socket(AF_INET, SOCK_STREAM, 0);
    if(sex->Socket < 0)
    {
      PrintSocketError("Failed to create socket");
      StopServer((Server*)sex);
      sex = 0;
      retval = NVN_ECONNFAIL;
    }

    if(sex)
    {
      memset(&serverAddr, 0, sizeof(serverAddr));
      serverAddr.sin_family = AF_INET;
      serverAddr.sin_port = htons(port);
      serverAddr.sin_addr.s_addr = INADDR_ANY;

      if(0 != bind(sex->Socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))
      {
        PrintSocketError("Failed to bind socket");
        StopServer((Server*)sex);
        sex = 0;
        retval = NVN_ECONNFAIL;
      }
    }

    if(sex)
    {
      if(0 != listen(sex->Socket, 1))
      {
        PrintSocketError("Failed to listen");
        StopServer((Server*)sex);
        sex = 0;
        retval = NVN_ECONNFAIL;
      }
    }

    if(sex)
    {
      sex->KeepServerAlive = 1;

#ifdef __WINDOWS__

      sex->ServerThread.Handle = 
        CreateThread(0, 0, ServerThreadEntryPoint, sex, 0, &sex->ServerThread.Id);

      if(0 >= sex->ServerThread.Handle)
#else

      if(0 != pthread_create(&sex->ThreadHandle, 0, ServerThreadEntryPoint, sex))
#endif
      {
        fprintf(stderr, "Failed to launch server thread\n");
        sex->KeepServerAlive = 0;
        StopServer((Server*)sex);
        sex = 0;
        retval = NVN_ETHREADFAIL;
      }
    }

    *server = (Server*)sex;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int StartServer(int port, ServerRequestCallback callback, ServerMode mode)
{
  int retval = NVN_NOERR;
  ServerEx* server = (ServerEx*)malloc(sizeof(ServerEx));
  struct sockaddr_in serverAddr;

  memset(server, 0, sizeof(ServerEx));
  server->Base.Port = port;
  server->Base.RequestCallback = callback;
  server->Base.Async = 0;
  server->Base.Mode = mode;

  server->Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server->Socket < 0)
  {
    PrintSocketError("Failed to create socket");
    FreeServerResources(server);
    server = 0;
    retval = NVN_ECONNFAIL;
  }

  if(server)
  {
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(0 != bind(server->Socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)))
    {
      PrintSocketError("Failed to bind socket");
      FreeServerResources(server);
      server = 0;
      retval = NVN_ECONNFAIL;
    }
  }

  if(server)
  {
    if(0 != listen(server->Socket, 1))
    {
      PrintSocketError("Failed to listen");
      FreeServerResources(server);
      server = 0;
      retval = NVN_ECONNFAIL;
    }
  }

  if(server)
  {
    server->KeepServerAlive = 1;
    retval = ServerLoop(server);
    FreeServerResources(server);
    server = 0;
  }

  return retval;
}

int SetReceiveBuffer(Server* server, char* buf, int len, int manage)
{
  int retval = NVN_NOERR;

  if(server)
  {
    if(server->ReceiveBuffer && server->ManageReceiveBuffer)
    {
      free(server->ReceiveBuffer);
      server->ReceiveBuffer = 0;
      server->ReceiveBufferLength = 0;
    }

    server->ReceiveBuffer = buf;
    server->ReceiveBufferLength = len;
    server->ManageReceiveBuffer = manage;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int SetSendBuffer(Server* server, char* buf, int len, int manage)
{
  int retval = NVN_NOERR;

  if(server)
  {
    if(server->SendBuffer && server->ManageSendBuffer)
    {
      free(server->SendBuffer);
      server->SendBuffer = 0;
      server->SendBufferLength = 0;
    }

    server->SendBuffer = buf;
    server->SendBufferLength = len;
    server->ManageSendBuffer = manage;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int StopServer(Server* server)
{
  int retval = NVN_NOERR;
  int iscur = 0;
  int valid = 0;

  if(server)
  {
    ServerEx* sex = (ServerEx*)server;
    sex->KeepServerAlive = 0;

    if(NVN_NOERR == ValidSocket(sex->Socket, &valid) && valid)
    {
      CloseSocket(sex->Socket);
      sex->Socket = 0;
    }

    if(sex->Base.Async && sex->ServerThreadAlive &&
#ifdef __WINDOWS__
       GetCurrentThreadId() == thread.Id)
#else
       pthread_equal(sex->ThreadHandle, pthread_self()))
#endif
    {
#ifdef __WINDOWS__
      WaitForSingleObject(sex->ServerThread.Handle, INFINITE);
      CloseHandle(sex->ServerThread.Handle);
      sex->ServerThread.Handle = 0;
      sex->ServerThread.Id = 0;
#else
      pthread_join(sex->ThreadHandle, 0);
#endif

      FreeServerResources(sex);
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int WaitAsyncServer(Server* server)
{
  int retval = NVN_NOERR;
  int iscur = 0;

  if(server)
  {
    ServerEx* sex = (ServerEx*)server;
#ifdef __WINDOWS__
    if(GetCurrentThreadId() == thread.Id)
#else
    if(pthread_equal(sex->ThreadHandle, pthread_self()))
#endif
    {
      fprintf(stderr, "Cannot wait for the current thread to finish");
      retval = NVN_ETHREADFAIL;
    }
    else
    {
#ifdef __WINDOWS__
      WaitForSingleObject(sex->ThreadHandle, INFINITE);
#else
      pthread_join(sex->ThreadHandle, 0);
#endif
    }
  }

  return retval;
}


/*****************************************************************************
 * Implementation of local functions:
 *****************************************************************************/

int CheckReceiveBuffer(ServerEx* server, int capacity)
{
  int retval = NVN_NOERR;

  if(server && capacity >= 0)
  {
    if(server->Base.ReceiveBufferLength <= capacity)
    {
      if(server->Base.ReceiveBuffer && server->Base.ManageReceiveBuffer)
        free(server->Base.ReceiveBuffer);
      
      server->Base.ReceiveBuffer = (char*)malloc(capacity + 1);
      server->Base.ReceiveBufferLength = capacity + 1;
      server->Base.ManageReceiveBuffer = 1;
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int CheckSendBuffer(ServerEx* server, int capacity)
{
  int retval = NVN_NOERR;

  if(server && capacity >= 0)
  {
    if(server->Base.SendBufferLength < capacity)
    {
      if(server->Base.SendBuffer && server->Base.ManageSendBuffer)
        free(server->Base.SendBuffer);

      server->Base.SendBuffer = (char*)malloc(capacity + 1);
      server->Base.SendBufferLength = capacity + 1;
      server->Base.ManageSendBuffer = 1;
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int FreeServerResources(ServerEx* server)
{
  int retval = NVN_NOERR;
  int valid = 0;

  if(server)
  {
    if(NVN_NOERR == ValidSocket(server->Socket, &valid) && valid)
      CloseSocket(server->Socket);

    if(server->Base.ReceiveBuffer && server->Base.ManageReceiveBuffer)
      free(server->Base.ReceiveBuffer);

    if(server->Base.SendBuffer && server->Base.ManageSendBuffer)
      free(server->Base.SendBuffer);

    free(server);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int HandleChattyClient(ServerEx* server, SOCKET acceptSocket,
                       const char* clientName)
{
  int retval = NVN_NOERR;
  int msgSize = 0;
  int received = 0;
  int sent = 0;
  int result = 0;
  int responseSize = 0;
  char logmsg[64];
  int valid = 0;

  if(server && NVN_NOERR == ValidSocket(acceptSocket, &valid) && valid)
  {
    if(NVN_NOERR != ReceiveMsgSize(acceptSocket, &msgSize))
    {
      fprintf(stderr, "Failed to receive message size\n");
      msgSize = 0;
    }

    if(msgSize > 0)
    {
      CheckReceiveBuffer(server, msgSize);

      server->Base.ReceiveBuffer[msgSize] = 0;
      while(NVN_NOERR == retval && received < msgSize)
      {
        result = recv(acceptSocket, 
                      server->Base.ReceiveBuffer + received, 
                      msgSize - received, 
                      0);
        
        if(result <= 0)
        {
          fprintf(stderr, "recv Failed\n");
          retval = NVN_ECOMMFAIL;
        }
        else
        {
          received += result;
        }
      }

      if(NVN_NOERR == retval)
      {
        CheckSendBuffer(server, 1024);

        if(msgSize == 4 && 0 == strcmp("ping", server->Base.ReceiveBuffer))
        {
          printf("Received ping\n");
          strcpy(server->Base.SendBuffer, "ping");
          responseSize = 4;
        }
        else
        {
          retval = InvokeCallback(server, clientName, msgSize, &responseSize);
          if(NVN_NOERR != retval)
            fprintf(stderr, "Server callback returned an error\n");
        }
      }

      if(NVN_NOERR == retval)
      {
        if(responseSize > 0)
        {
          SendMsgSize(acceptSocket, responseSize);
          
          while(NVN_NOERR == retval && sent < responseSize)
          {
            result = send(acceptSocket, 
                          server->Base.SendBuffer + sent, 
                          responseSize - sent, 
                          0);
            
            if(result <= 0)
            {
              PrintSocketError("send failed");
              retval = NVN_ECOMMFAIL;
            }
            else
            {
              sent += result;
            }
          }
          
          if(NVN_NOERR != retval)
            fprintf(stderr, "Error sending response\n");
        }
      }
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int HandleStreamClient(ServerEx* server, SOCKET acceptSocket,
                       const char* clientName)
{
  int retval = NVN_NOERR;
  int msgSize = 0;
  int received = 0;
  int result = 0;
  int keepConnectionAlive = 1;
  char logmsg[64];
  int valid = 0;

  while(server && server->KeepServerAlive && keepConnectionAlive &&
        NVN_NOERR == ValidSocket(acceptSocket, &valid) && valid)
  {
    result = ReceiveMsgSize(acceptSocket, &msgSize);
    if(NVN_ECLIENTGONE == result)
    {
      printf("Client disconnected.\n");
      keepConnectionAlive = 0;
    }
    else if(NVN_NOERR != result)
    {
      retval = NVN_ECOMMFAIL;
      fprintf(stderr, "Failed to receive message size\n");
      msgSize = 0;
    }
    else if(msgSize <= 0)
    {
      fprintf(stderr, "Invalid message size: %d\n", msgSize);
      retval = NVN_ECOMMFAIL;
    }
    else
    {
      CheckReceiveBuffer(server, msgSize);

      received = 0;
      server->Base.ReceiveBuffer[msgSize] = 0;
      while(keepConnectionAlive && retval == NVN_NOERR && received < msgSize)
      {
        result = recv(acceptSocket, 
                      server->Base.ReceiveBuffer + received, 
                      msgSize - received, 
                      0);
        
        if(result < 0)
        {
          PrintSocketError("recv failed");
          keepConnectionAlive = 0;
          retval = NVN_ECOMMFAIL;
        }
        else if(result == 0)
        {
          printf("Client disconnected.\n");
          keepConnectionAlive = 0;
        }
        else
        {
          received += result;
        }
      }

      InvokeCallback(server, clientName, msgSize, 0);
    }
  }

  return retval;
}

int InvokeCallback(ServerEx* server, const char* clientName, int recvSize, int* responseSize)
{
  int retval = 0;
  int respsize = 0;

  if(server)
  {
    respsize = server->Base.SendBufferLength;
    retval = server->Base.RequestCallback((Server*)server, 
                                          clientName,
                                          server->Base.ReceiveBuffer, 
                                          recvSize,
                                          server->Base.SendBuffer, 
                                          &respsize);

    if(NVN_NOERR == retval && responseSize)
      *responseSize = respsize;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int ServerLoop(ServerEx* server)
{
  int retval = NVN_NOERR;

  struct sockaddr_in clientAddr;
  int clientAddrLen = sizeof(clientAddr);
  char addr[32];
  SOCKET acceptSocket = INVALID_SOCKET;
  char logmsg[256];
  int valid = 0;

  if(server)
  {
    while(NVN_NOERR == ValidSocket(server->Socket, &valid) && valid && 
          server->KeepServerAlive)
    {
      acceptSocket = accept(server->Socket, 
                            (struct sockaddr*)&clientAddr, 
                            &clientAddrLen);

      if(NVN_NOERR == ValidSocket(acceptSocket, &valid) && valid)
      {
#ifdef __WINDOWS__
        InetNtop(AF_INET, &clientAddr.sin_addr, addr, INET_ADDRSTRLEN);
#else
        inet_ntop(AF_INET, &clientAddr.sin_addr, addr, INET_ADDRSTRLEN);
#endif

        if(ServerModeChatty == server->Base.Mode)
          HandleChattyClient(server, acceptSocket, addr);
        else
          HandleStreamClient(server, acceptSocket, addr);

        CloseSocket(acceptSocket);
      }
      else
      {
        fprintf(stderr, "Failed to accept connection\n");
      }
    }
  }

  return retval;
}

#ifdef __WINDOWS__
DWORD WINAPI ServerThreadEntryPoint(LPVOID arg)
#else
void* ServerThreadEntryPoint(void* arg)
#endif
{
  ServerEx* sex = (ServerEx*)arg;
  if(sex)
  {
    sex->ServerThreadAlive = 1;
    ServerLoop(sex);
    sex->ServerThreadAlive = 0;
  }

  return 0;
}
