/**
  server.h - Created by Timothy Morey on 4/5/2012.
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#include "nvn.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct Server;

/**
  Defines a function pointer that is used so that the server may notify the
  system when it receives a request.

  @param[in] server The server that received the request.
  @param[in] clientName The ip address of the client in dotted decimal notation.
  @param[in] recvBuf The request data buffer.
  @param[in] recvLen The request length.
  @param[in,out] replyBuf The buffer that may be used to store the response.
  @param[in,out] replyLen The length of the replyBuf.  When the function is 
  first invoked, this will store the total available length of the replyBuf.
  When the function returns, this should contain the number of bytes in 
  replyBuf that should be sent to the client.  If this value is set to 0, no
  response will be sent.
  
  @return An nvn error code indicating the success of the operation.  If the
  return value is NVN_NOERR and the reply buffer is nonempty, a response will
  be sent.
*/
typedef int (*ServerRequestCallback)(struct Server* server,
                                     const char* clientName,
                                     const char* recvBuf, int recvLen, 
                                     char* replyBuf, int* replyLen);

/**
  Enumerates the supported server types.
*/
typedef enum
{
  ServerModeChatty,  // A chatty server is one that receives requests and
                     // responds to each one.

  ServerModeStream   // A stream server simply receives data and never
                     // responds.

} ServerMode;

/**
  A server is a component that listens to a port, receives connections from 
  clients, and optionally responds to them.
*/
typedef struct Server
{
  int Async;                // Indicates if the server operates on the same
                            // thread it is created on, or if it operates on a
                            // background thread that it manages.

  ServerMode Mode;          // Indicates if this is a chatty or stream server.

  int Port;                 // The port that the server listens on.

  ServerRequestCallback RequestCallback;
                            // A pointer to a function that will be called each
                            // time the server receives a message.

  char* ReceiveBuffer;      // The buffer that is used to store incoming data.

  int ReceiveBufferLength;  // The maximum capacity of ReceiveBuffer, in bytes.

  int ManageReceiveBuffer;  // Indicates if the server manages the memory in 
                            // the ReceiveBuffer.

  char* SendBuffer;         // The buffer that is used to store responses.

  int SendBufferLength;     // The maximum capacity of SendBuffer, in bytes.

  int ManageSendBuffer;     // Indicates if the server manages the memory in
                            // the SendBuffer.

} Server;


/**
  Creates a new server and starts a thread to listen for connections.  The
  server will continue to loop on a background thread until StopServer is 
  called.

  @param[in] port The port through which the server will accept connections.
  @param[in] requestCallback A pointer to a function that will handle requests
  that the server receives.  This function will be called from the server 
  thread.
  @param[in] mode Indicates if the server should be a chatty or stream server.
  @param[out] server A pointer to the newly constructed server.
  
  @return An nvn error code indicating if the operation was successful.
*/
int StartAsyncServer(int port, ServerRequestCallback callback, ServerMode mode, Server** server);

/**
  Creates a new server and starts listening for connections on the current 
  thread.  This function will not return until StopServer is called, and the 
  server will run on the calling thread.

  @param[in] port The port through which the server will accept connections.
  @param[in] requestCallback A pointer to a function that will handle requests that
  the server receives.  This function will be called from the thread that calls
  StartServer.
  @param[in] Indicates if the server should be a chatty or stream server.
  
  @return An nvn error code indicating if the operation was successful.
*/
int StartServer(int port, ServerRequestCallback callback, ServerMode mode);

/**
  Forces the server to use the specified buffer for incoming data.  If the
  buffer is not large enough for an incoming message, it will be freed and 
  replaced by a sufficiently large buffer.  
  
  This is the buffer containing the incoming data that is passed to the callback
  function when a request comes in.

  @param[in] server The server for which a buffer is being provided.
  @param[in] buf The buffer that will be used to hold incoming data for the
  server.
  @param[in] len The size of buf.
  @param[in] manage Indicates if the server should manage the memory associated
  with the buffer and free it when it is no longer needed, or if it should
  assume that someone else is managing the memory.
  
  @return An nvn error code indicating if the operation was successful.
*/
int SetReceiveBuffer(Server* server, char* buf, int len, int manage);

/**
  Forces the server to use the specified buffer for outgoing data.  If the
  buffer is not large enough for an incoming message, it will be freed and 
  replaced by a sufficiently large buffer.

  This buffer is passed to the request callback function when a request occurs,
  so that the handler may provide a reply.

  @param[in] server The server for which a buffer is being provided.
  @param[in] buf The buffer that will be used to hold incoming data for the 
  server.
  @param[in] len The size of buf.
  @param[in] manage Indicates if the server should manage the memory associated
  with the buffer and free it when it is no longer needed, or if it should
  assume that someone else is managing the memory.
  
  @return An nvn error code indicating if the operation was successful.
*/
int SetSendBuffer(Server* server, char* buf, int len, int manage);

/**
  Stops the specified server and releases all associated resources.  If this
  function is called from the server thread, it will initiate the server
  stopping and return, so that the server loop may cleanly terminate.  If this
  function is called from another thread, the calling thread will block until
  the server thread has terminated and the server is fully destroyed.

  @param[in] server The server to be stopped.
  
  @return An nvn error code indicating if the operation was successful.
*/
int StopServer(Server* server);

int WaitAsynServer(Server* server);


#ifdef __cplusplus
}
#endif

#endif
