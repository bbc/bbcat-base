#ifndef __UDP_SOCKET__
#define __UDP_SOCKET__

#include "misc.h"

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** VERY, VERY simple UDP socket object
 *
 */
/*--------------------------------------------------------------------------------*/
class UDPSocket
{
public:
  UDPSocket();
  ~UDPSocket();

  /*--------------------------------------------------------------------------------*/
  /** Bind UDP socket to an address and port for receiving
   *
   * @param bindaddress bind address (usually 0.0.0.0 or 127.0.0.1)
   * @param port port number of listen on
   *
   * @return true for success
   */
  /*--------------------------------------------------------------------------------*/
  bool   bind(const char *bindaddress, uint_t port);

  /*--------------------------------------------------------------------------------*/
  /** Bind UDP socket to a port for receiving
   *
   * @param port port number of listen on
   *
   * @return true for success
   */
  /*--------------------------------------------------------------------------------*/
  bool   bind(uint_t port) {return bind("0.0.0.0", port);}

  /*--------------------------------------------------------------------------------*/
  /** Connect UDP port to remote address and port
   *
   * @param address remote address to connect to
   * @param port remote port number to connect to
   *
   * @return true for success
   */
  /*--------------------------------------------------------------------------------*/
  bool   connect(const char *address, uint_t port);

  /*--------------------------------------------------------------------------------*/
  /** Close UDP socket
   */
  /*--------------------------------------------------------------------------------*/
  void   close();

  /*--------------------------------------------------------------------------------*/
  /** Return whether a socket is open
   */
  /*--------------------------------------------------------------------------------*/
  bool   isopen() const {return (socket >= 0);}

  /*--------------------------------------------------------------------------------*/
  /** Wait for something to happen the UDP socket or timeout
   */
  /*--------------------------------------------------------------------------------*/
  bool   wait(uint_t timeout = 0);

  bool   send(const void *data, uint_t bytes);
  sint_t recv(void *data, uint_t maxbytes);

protected:
  int  socket;
  void *readfds;
};

BBC_AUDIOTOOLBOX_END

#endif
