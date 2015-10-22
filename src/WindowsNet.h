#ifndef __WINDOWS_NET__
#define __WINDOWS_NET__

#include "misc.h"

#include <ws2tcpip.h>

BBC_AUDIOTOOLBOX_START

/*--------------------------------------------------------------------------------*/
/** Windows network support initialisation
 *
 * Any objects using networking MUST call WindowsNet::Init() (on Windows) before trying any network operations!
 */
/*--------------------------------------------------------------------------------*/
class WindowsNet
{
public:
  ~WindowsNet() {
    ::WSACleanup();
  }

  static void Init() {
    static WindowsNet __net;
    UNUSED_PARAMETER(__net);
  }
  
protected:
  WindowsNet() {
    ::WSAStartup(MAKEWORD(2, 0), &WSAData);
  }

protected:
  WSADATA WSAData;
};

BBC_AUDIOTOOLBOX_END

#endif
