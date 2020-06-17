#ifndef __DEFINESANDGVAR_H__
#define __DEFINESANDGVAR_H__
/*
*	Filename: definesGvars.h
*	This file contains all the defines and global variable
*	Author: venkat Nalla
*	Date: Mar 2017
*/
/*
//Occasionally these macros are not being defined/maintained. 
//So, test the below macros to see which one is defined and use one of it to 
//define the _POSIX
#if defined __linux__
	#pragma message("__linux__ defined")
#endif
#if defined __gnu_linux__
	#pragma message("__gnu_linux__ defined")
#endif
#if defined __unix__
	#pragma message("__unix__ defined")
#endif
*/
#if defined __linux__ && (!defined _POSIX)
	#define _POSIX
//	#pragma message "_POSIX defined"
#endif


#if defined(WINSOCK2)
	#pragma message("abcd")
#include <winsock2.h>
#elif defined(_POSIX)
//	#pragma message("xyz")
	#include <sys/types.h>
//	#include <sys/socket.h>
//	#include <netinet/in.h>
//	#include <arpa/inet.h>
//	#include <sys/time.h>
//	#include <netdb.h>
	#include <sys/ioctl.h>	// For FIONBIO
	#include <net/if.h>	// For IFF_LOOPBACK, ifconf, ifreq
//	#include <sys/utsname.h>
	#include <netinet/in.h>
	#include <unistd.h>
	#include <errno.h>
	#include <semaphore.h>
	#include <malloc.h>
	#include <fcntl.h>
	#include <signal.h>
	#include <string.h>
//	#include <termios.h>
//	#include <pthread.h>
	#include <stdlib.h> // for exit()
	#include <stdio.h>  // For printf
	//#include <stddef.h>
	#include <string.h> // for strcpy, strlen, memcpy etc
	//#include <math.h>
	#include <termios.h>	// For ICANON, ECHO, TCSANOW etc
	//#include <unistd.h>
	//#include <sys/select.h> // for 
	#include <pthread.h>

	#define DEBUGSTATEMENT(x)  
#endif
#define PORT 1150
#define BUFFERSIZE 1024*1024	// 8192

#ifdef _POSIX
	#define FALSE	0
	#define TRUE	1
	typedef int BOOL;
	typedef unsigned int WORD;
	typedef unsigned long DWORD;
//	typedef void*	HANDLE;
	typedef int SOCKET;
	#define INVALID_SOCKET	(SOCKET)(~0)
	#define SOCKET_ERROR	(-1)
	#define INFINITE 	0xFFFFFFFF
	#define FAR	far
	typedef struct _WSABUF {
		DWORD      len;     // the length of the buffer 
		char  *  buf;     // the pointer to the buffer 
	} WSABUF,  * LPWSABUF;

	int ioctlsocket(SOCKET s, DWORD z,DWORD *flag);
#endif // _POSIX

typedef struct _MYSOCKET_INFORMATION {
   char Buf[BUFFERSIZE];
   WSABUF wsaBuf;
   SOCKET s;
//   OVERLAPPED Overlapped;
   DWORD txb;
   DWORD rxb;
   int   index;
} SOCKET_INFO, * LPSOCKET_INFO;

typedef enum 
{
	GET_USER,
	GET_PWD,
	READY,
	GET_FILE,
	WRONGID
}state;

#endif //__DEFINESANDGVAR_H__

