/**
   socket-utils.h - Created by Timothy Morey on 4/18/2012.

   This file defines some cross-platform functions for working with sockets.
*/

#ifndef __SOCKET_UTILS_H
#define __SOCKET_UTILS_H

#include "nvn.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __WINDOWS__
#define INVALID_SOCKET -1
typedef int SOCKET;
#endif

/**
  @return An nvn error code indicating if the operation was successful.
*/
int CloseSocket(SOCKET socket);

/**
  Checks the system to find the last error (errno on linux and WSAGetLastError
  on windows), and prints a message to stderr about it.

  @param[in] msg A custom message that will be included in the error report.

  @return An nvn error code indicating if the operation was successful.
 */
int PrintSocketError(const char* msg);

/**
  Receives the next four bytes from the specified socket, assuming that the
  give the size of the following message, and returns their integer value.

  @param[in] socket The socket that will be used to receive four bytes.
  @param[out] size The first four bytes read from the socket, interpreted as an
  integer.

  @return An nvn error code indicating if the operation was successful.
*/
int ReceiveMsgSize(SOCKET socket, int* size);

/**
  Sends four bytes through the specified socket, which contain the value of the
  size parameter.

  @param[in] socket The socket through which four bytes will be sent.
  @param[in] size The value that will be sent through the socket.

  @return An nvn error code indicating if the operation was successful.
*/
int SendMsgSize(SOCKET socket, int size);

/**
  Indicates if socket is a potentially valid socket handle.

  @param[in] socket The socket handle that will be tested.
  @param[out] valid 1 if socket looks valid and 0 otherwise.

  @return An nvn error code indicating if the operation was successful.
*/
int ValidSocket(SOCKET socket, int* valid);

#ifdef __cplusplus
}
#endif

#endif
