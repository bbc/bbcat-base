
#include <string.h>

#include "OSCompiler.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifndef COMPILER_MSVC
#include <sys/param.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <limits.h>
#include <signal.h>

#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define DEBUG_LEVEL 1
#include "UDPSocket.h"

#ifdef TARGET_OS_WINDOWS
#include "WindowsNet.h"
#endif

BBC_AUDIOTOOLBOX_START

static bool resolve(const char *address, uint_t port, struct sockaddr_in* sockaddr)
{
  bool success = false;

#ifdef TARGET_OS_WINDOWS
  // ensure Windows networking initialised
  WindowsNet::Init();
#endif

  memset(sockaddr, 0, sizeof(*sockaddr));
  sockaddr->sin_family = AF_INET;
  sockaddr->sin_port = htons((short) (port & 0xFFFF));

  sockaddr->sin_addr.s_addr = inet_addr(address);

  if (sockaddr->sin_addr.s_addr == INADDR_NONE)
  {
    struct hostent *hp = gethostbyname(address);

    if (hp)
    {
      memcpy(&sockaddr->sin_addr, hp->h_addr, hp->h_length);
      sockaddr->sin_family = hp->h_addrtype;
      success = true;
    }
    else debug_err("Host '%s' not found", address);
  }
  else success = true;

  return success;
}

UDPSocket::UDPSocket() : socket(-1)
{
#ifdef TARGET_OS_WINDOWS
  // ensure Windows networking initialised
  WindowsNet::Init();
#endif
  
  readfds = (void *)new fd_set;
}

UDPSocket::~UDPSocket()
{
  close();

  if (readfds) delete (fd_set *)readfds;
}

bool UDPSocket::bind(const char *bindaddress, uint_t port)
{
  struct sockaddr_in sockaddr;
  bool success = false;

  if (!resolve(bindaddress, port, &sockaddr)) return success;

  if ((socket = (int)::socket(sockaddr.sin_family, SOCK_DGRAM, 0)) >= 0)
  {
    int rc = 1;

    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));

    if (::bind(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0)
    {
      success = true;
    }
    else debug_err("Failed to bind to %s:%u (%s)", bindaddress, port, strerror(errno));
  }
  else debug_err("Failed to create socket (%s)", strerror(errno));

  if (!success) close();

  return success;
}

bool UDPSocket::connect(const char *address, uint_t port)
{
  struct sockaddr_in sockaddr;
  bool success = false;

  if (!resolve(address, port, &sockaddr)) return success;

  if ((socket = (int)::socket(sockaddr.sin_family, SOCK_DGRAM, 0)) >= 0)
  {
    if (::connect(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0)
    {
      success = true;
    }
    else debug_err("Failed to connect to %s:%u (%s)", address, port, strerror(errno));
  }
  else debug_err("Failed to create socket (%s)", strerror(errno));
  
  if (!success) close();

  return success;
}

void UDPSocket::close()
{
  // close down socket
  if (socket >= 0)
  {
#ifdef COMPILER_MSVC
	closesocket(socket);
#else
    ::close(socket);
#endif
    socket = -1;
  }
}

bool UDPSocket::send(const void *data, uint_t bytes)
{
  bool success = false;

  if (socket >= 0)
  {
    success = (::send(socket, (const char*)data, bytes, 0) >= 0);
    if (!success) debug_err("Failed to send %u bytes to socket (%s)", bytes, strerror(errno));
  }

  return success;
}

sint_t UDPSocket::recv(void *data, uint_t maxbytes)
{
  sint_t bytes = -1;

  if (socket >= 0)
  {
    // recv() with MSG_PEEK doesn't work propely unless *some* buffer is supplied so this buffer is just used
    // as a dumping point for the data
    // it *still* means recv()  is thread-safe because we don't care about the data
    static char _staticbuf[16384];
    
    bytes = ::recv(socket, data ? (char *)data : _staticbuf, data ? maxbytes : sizeof(_staticbuf), data ? 0 : MSG_PEEK);
    
    if (bytes < 0) debug_err("Failed to receive %u bytes from socket (%s)", maxbytes, strerror(errno));
  }

  return bytes;
}

/*--------------------------------------------------------------------------------*/
/** Wait for something to happen the UDP socket or timeout
 */
/*--------------------------------------------------------------------------------*/
bool UDPSocket::wait(uint_t timeout)
{
  bool success = false;

  if (socket >= 0)
  {
    struct timeval tv;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    FD_ZERO((fd_set *)readfds);
    FD_SET(socket, (fd_set *)readfds);

    if (::select(socket + 1, (fd_set *)readfds, NULL, NULL, &tv) >= 0)
    {
      success = (FD_ISSET(socket, (fd_set *)readfds) != 0);
    }
  }

  return success;
}

BBC_AUDIOTOOLBOX_END
