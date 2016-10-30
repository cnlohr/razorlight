#ifdef WIN32

#include <winsock2.h>
#include <windows.h>

int GetMyIP( char * tos, int sizeoftos, int index )
{
    char ac[80];
	int i;
    struct WSAData wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        return -1;
    }

    if (gethostname(ac, sizeof( ac ) ) == SOCKET_ERROR) {
        return -1;
    }

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        return -1;
    }

    for (i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
		if( i == index )
		{
        	memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			strncpy( tos, inet_ntoa( addr ), sizeoftos );
			return 0;
		}
    }
	return -2;
}


#else

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

int GetMyIP( char * tos, int sizeoftos, int index )
{
	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;
	int i = 0;
	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if( strncmp( addressBuffer, "127", 3 ) == 0 ) continue;
			if( i == index )
			{
				strncpy( tos, addressBuffer, sizeoftos );
				if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
				return 0;
			}
			i++;
//			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
/*		} else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
			// is a valid IP6 Address
			tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
*/
		} 
	}
	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return -1;
}

#endif

