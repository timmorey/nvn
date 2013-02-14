/**
   socket-utils.c - Created by Timothy Morey on 4/18/2012.

   This file implements the cross-platform functions for working with sockets
   defined in socket-utils.h
 */


#include "nvn.h"

#include "socket-utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef __WINDOWS__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#endif

#ifdef __WINDOWS__
#pragma warning(disable:4996)
#endif


int CloseSocket(SOCKET socket)
{
  int retval = NVN_NOERR;

#ifdef __WINDOWS__
  if(0 != shutdown(socket, SD_BOTH) ||
      0 != closesocket(socket))
#else
    if(0 != shutdown(socket, SHUT_RDWR) ||
        0 != close(socket))
#endif
      PrintSocketError("Failed to close socket");

  return retval;
}

int PrintSocketError(const char* msg)
{
  int retval = NVN_NOERR;
  int err;
  char errStr[256];
  char logmsg[256];

#ifdef __WINDOWS__
  err = WSAGetLastError();
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, errStr, 256, 0);
#else
  err = errno;
  strerror_r(err, errStr, 256);
#endif

  fprintf(stderr, "SocketError: %s.  errno=%d: %s", msg, err, errStr);

  return retval;
}

int ReceiveMsgSize(SOCKET socket, int* size)
{
  int retval = NVN_NOERR;
  int valid = 0;

  if(NVN_NOERR == ValidSocket(socket, &valid) && valid && size)
  {
    char buf[4];
    int received = 0;
    int result = 0;

    while(received < 4)
    {
      result = recv(socket, buf + received, 4 - received, 0);
      if(result <= 0)
      {
        PrintSocketError("recv failed");
        retval = NVN_ECOMMFAIL;
        break;
      }
      else
      {
        received += result;
      }
    }

    if(received == 4)
    {
      if(sizeof(int) == 4)
      {
        *size = *((int*)buf);
        *size = ntohl(*size);
      }
      else
      {
        retval = NVN_ERROR;
        fprintf(stderr, "Architecture not supported: sizeof(int) != 4\n");
      }
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int SendMsgSize(SOCKET socket, int size)
{
  int retval = NVN_NOERR;
  char buf[4];
  int sent = 0;
  int result = 0;
  int valid = 0;

  if(NVN_NOERR == ValidSocket(socket, &valid) && valid)
  {
    if(sizeof(int) == 4)
    {
      ((int*)buf)[0] = htonl(size);
    }
    else
    {
      retval = NVN_ERROR;
      fprintf(stderr, "Architecture not supported: sizeof(int) != 4\n");
    }

    while(sent < 4)
    {
      result = send(socket, buf + sent, 4 - sent, 0);
      if(result <= 0)
      {
        retval = NVN_ECOMMFAIL;
        PrintSocketError("send failed");
        break;
      }
      else
      {
        sent += result;
      }
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int ValidSocket(SOCKET socket, int* valid)
{
  int retval = NVN_NOERR;

  if(valid)
  {
#ifdef __WINDOWS__
    *valid = socket != INVALID_SOCKET;
#else
    *valid = socket >= 0;
#endif
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}
