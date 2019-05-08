// net_common.cpp : Common functions for the network system.

#include "net_common.h"

/***********/
/* Globals */
/***********/

/**********/
/* Public */
/**********/

//////////////////////////////////////////////////////////////////////////
// Initialize the network system (Only need to call this once).
//
// Parameters
// None.
//
// Returns
// Bool     TRUE if OK.
//////////////////////////////////////////////////////////////////////////
bool NET_Init(void)
{
    // Contains the details of the Winsock implementation.
    WSADATA wsadata;
    int     iStatus;
    
    // Init the current version (2.2) of winsock.
    iStatus = WSAStartup(WINSOCK_VERSION, &wsadata);

    if (iStatus)
    { 
        // Failed to load the winsock .dll.
        return false;
    }
    return true;
}
                
//////////////////////////////////////////////////////////////////////////
// Free all network resources.
//
// Parameters
// None.
//
// Returns
// None.
//////////////////////////////////////////////////////////////////////////
void NET_ShutDown(void)
{
    WSACleanup();
}

//////////////////////////////////////////////////////////////////////////
// Gets the last error that occurred.
//
// Parameters
// None.
//
// Returns
// Int32    The error number.
//////////////////////////////////////////////////////////////////////////
int NET_GetErrorNumber(void)
{
    return WSAGetLastError();
}

//////////////////////////////////////////////////////////////////////////
// Gets the last error that occured in string format.
//
// Parameters
// None.
//
// Returns
// Char *   The error in string format.
//////////////////////////////////////////////////////////////////////////
const char *NET_GetErrorString(void)
{
    int iErrCode = NET_GetErrorNumber();

    // Case constants defined in <winsock2.h>.
    switch (iErrCode)
    {
        case WSAEINTR:                return "WSAEINTR";
        case WSAEBADF:                return "WSAEBADF";
        case WSAEACCES:               return "WSAEACCES";
        case WSAEFAULT:               return "WSAEFAULT";
        case WSAEINVAL:               return "WSAEINVAL";
        case WSAEMFILE:               return "WSAEMFILE";
        case WSAEWOULDBLOCK:          return "WSAEWOULDBLOCK";
        case WSAEINPROGRESS:          return "WSAEINPROGRESS";
        case WSAEALREADY:             return "WSAEALREADY";
        case WSAENOTSOCK:             return "WSAENOTSOCK";
        case WSAEDESTADDRREQ:         return "WSAEDESTADDRREQ";
        case WSAEMSGSIZE:             return "WSAEMSGSIZE";
        case WSAEPROTOTYPE:           return "WSAEPROTOTYPE";
        case WSAENOPROTOOPT:          return "WSAENOPROTOOPT";
        case WSAEPROTONOSUPPORT:      return "WSAEPROTONOSUPPORT";
        case WSAESOCKTNOSUPPORT:      return "WSAESOCKTNOSUPPORT";
        case WSAEOPNOTSUPP:           return "WSAEOPNOTSUPP";
        case WSAEPFNOSUPPORT:         return "WSAEPFNOSUPPORT";
        case WSAEAFNOSUPPORT:         return "WSAEAFNOSUPPORT";
        case WSAEADDRINUSE:           return "WSAEADDRINUSE";
        case WSAEADDRNOTAVAIL:        return "WSAEADDRNOTAVAIL";
        case WSAENETDOWN:             return "WSAENETDOWN";
        case WSAENETUNREACH:          return "WSAENETUNREACH";
        case WSAENETRESET:            return "WSAENETRESET";
        case WSAECONNABORTED:         return "WSAECONNABORTED";
        case WSAECONNRESET:           return "WSAECONNRESET";
        case WSAENOBUFS:              return "WSAENOBUFS";
        case WSAEISCONN:              return "WSAEISCONN";
        case WSAENOTCONN:             return "WSAENOTCONN";
        case WSAESHUTDOWN:            return "WSAESHUTDOWN";
        case WSAETOOMANYREFS:         return "WSAETOOMANYREFS";
        case WSAETIMEDOUT:            return "WSAETIMEDOUT";
        case WSAECONNREFUSED:         return "WSAECONNREFUSED";
        case WSAELOOP:                return "WSAELOOP";
        case WSAENAMETOOLONG:         return "WSAENAMETOOLONG";
        case WSAEHOSTDOWN:            return "WSAEHOSTDOWN";
        case WSAEHOSTUNREACH:         return "WSAEHOSTUNREACH";
        case WSAENOTEMPTY:            return "WSAENOTEMPTY";
        case WSAEPROCLIM:             return "WSAEPROCLIM";
        case WSAEUSERS:               return "WSAEUSERS";
        case WSAEDQUOT:               return "WSAEDQUOT";
        case WSAESTALE:               return "WSAESTALE";
        case WSAEREMOTE:              return "WSAEREMOTE";
        case WSASYSNOTREADY:          return "WSASYSNOTREADY";
        case WSAVERNOTSUPPORTED:      return "WSAVERNOTSUPPORTED";
        case WSANOTINITIALISED:       return "WSANOTINITIALISED";
        case WSAEDISCON:              return "WSAEDISCON";
        case WSAENOMORE:              return "WSAENOMORE";
        case WSAECANCELLED:           return "WSAECANCELLED";
        case WSAEINVALIDPROCTABLE:    return "WSAEINVALIDPROCTABLE";
        case WSAEINVALIDPROVIDER:     return "WSAEINVALIDPROVIDER";
        case WSAEPROVIDERFAILEDINIT:  return "WSAEPROVIDERFAILEDINIT";
        case WSASYSCALLFAILURE:       return "WSASYSCALLFAILURE";
        case WSASERVICE_NOT_FOUND:    return "WSASERVICE_NOT_FOUND";
        case WSATYPE_NOT_FOUND:       return "WSATYPE_NOT_FOUND";
        case WSA_E_NO_MORE:           return "WSA_E_NO_MORE";
        case WSA_E_CANCELLED:         return "WSA_E_CANCELLED";
        case WSAEREFUSED:             return "WSAEREFUSED";
        case WSANO_DATA:              return "WSANO_DATA";
        default:                      return "UNKNOWN_ERROR";
    }
}