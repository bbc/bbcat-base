
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <limits.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define DEBUG_LEVEL 1
#include "UDPSocket.h"

BBC_AUDIOTOOLBOX_START

static bool resolve(const char *address, uint_t port, struct sockaddr_in* sockaddr)
{
  bool success = false;

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
    else ERROR("Host '%s' not found", address);
  }
  else success = true;

  return success;
}

UDPSocket::UDPSocket() : socket(-1)
{
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

  if ((socket = ::socket(sockaddr.sin_family, SOCK_DGRAM, 0)) >= 0)
  {
    int rc = 1;

    setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));

    if (::bind(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0)
    {
      success = true;
    }
    else ERROR("Failed to bind to %s:%u (%s)", bindaddress, port, strerror(errno));
  }
  else ERROR("Failed to create socket (%s)", strerror(errno));

  if (!success) close();

  return success;
}

bool UDPSocket::connect(const char *address, uint_t port)
{
  struct sockaddr_in sockaddr;
  bool success = false;

  if (!resolve(address, port, &sockaddr)) return success;

  if ((socket = ::socket(sockaddr.sin_family, SOCK_DGRAM, 0)) >= 0)
  {
    if (::connect(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) >= 0)
    {
      success = true;
    }
    else ERROR("Failed to connect to %s:%u (%s)", address, port, strerror(errno));
  }
  else ERROR("Failed to create socket (%s)", strerror(errno));
  
  if (!success) close();

  return success;
}

void UDPSocket::close()
{
  // close down socket
  if (socket >= 0)
  {
    ::close(socket);
    socket = -1;
  }
}

bool UDPSocket::send(const void *data, uint_t bytes)
{
  bool success = false;

  if (socket >= 0)
  {
    success = (::send(socket, data, bytes, 0) >= 0);
    if (!success) ERROR("Failed to send %u bytes to socket (%s)", bytes, strerror(errno));
  }

  return success;
}

sint_t UDPSocket::recv(void *data, uint_t maxbytes)
{
  sint_t bytes = -1;

  if (socket >= 0)
  {
    bytes = ::recv(socket, data, maxbytes, data ? 0 : MSG_PEEK);
    if (bytes < 0) ERROR("Failed to receive %u from socket (%s)", maxbytes, strerror(errno));
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
      success = FD_ISSET(socket, (fd_set *)readfds);
    }
  }

  return success;
}

BBC_AUDIOTOOLBOX_END
