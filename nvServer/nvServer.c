// Author: Venkat Nalla
// This is simple test program to test the latency of transferring a file using direct TCP/IP connection
// nvServer.c : Defines the entry point for the console application.
//
#include "definesGvars.h"

#if defined(WIN32)
	#include "stdafx.h"
	#include <winsock2.h>
	#include <conio.h>
#elif defined(_POSIX)


#endif


BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);
// BOOL serverStatus = FALSE;
DWORD TotalSockets = 0;
LPSOCKET_INFO SocketList[FD_SETSIZE];
static state currState = GET_USER;
//static state currState = READY;

#if defined(_POSIX)
int ioctlsocket(SOCKET s, DWORD z,DWORD *f)
{
   WORD flag;
   int result;
   
   flag = fcntl(s, F_GETFL,0);
   printf("ioctlsocket(): State before %s: 0x%x\n", (*f == 0) ? "block" : "nonblock", flag);
   if (*f == 0) // 0 means block
   {
  	printf("ioctlsocket(): State before block: 0x%X\n", flag);
	result = fcntl(s, F_SETFL,~O_NONBLOCK&flag);
   }
   else
   {
  	printf("ioctlsocket(): State before nonblock: 0x%X\n", flag);
	result = fcntl(s, F_SETFL,O_NONBLOCK|flag);
    }
   flag = fcntl(s, F_GETFL,0);
   printf("ioctlsocket(): State after %s: 0x%x\n", (*f == 0) ? "block" : "nonblock", flag);
   if (result != 0)
	result = -1;
   return result;
}

int _getch( ) 
{
   struct termios oldt, newt;
   int ch;
   
   tcgetattr( STDIN_FILENO, &oldt );
   newt = oldt;
   newt.c_lflag &= ~( ICANON | ECHO );
   tcsetattr( STDIN_FILENO, TCSANOW, &newt );
   ch = getchar();
   tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
   return ch;
}
int _kbhit(void)
{
  struct timeval tv;
  fd_set read_fd;

  /* Do not wait at all, not even a microsecond */
  tv.tv_sec=0;
  tv.tv_usec=0;

  /* Must be done first to initialize read_fd */
  FD_ZERO(&read_fd);

  /* Makes select() ask if input is ready:
   * 0 is the file descriptor for stdin    */
  FD_SET(0,&read_fd);

  /* The first parameter is the number of the
   * largest file descriptor to check + 1. */
  if(select(1, &read_fd,
            NULL, /* No writes */
            NULL, /* No exceptions */
            &tv)
     == -1)
    return 0;  /* An error occured */

  /*  read_fd now holds a bit map of files that are
   * readable. We test the entry for the standard
   * input (file 0). */
  if(FD_ISSET(0,&read_fd))
    /* Character pending on stdin */
    return 1;

  /* no characters were pending */
  return 0;
}


#endif


DWORD getIPaddress(void)
{
	DWORD	retValue;
	int attemptsLeft;
	int i, index;
	char inBuf[128];
	struct  in_addr addr;
#if defined(WINSOCK2)
	char buffer[16*sizeof(SOCKET_ADDRESS_LIST)];
	DWORD bufferSize;

	attemptsLeft = 3;
	SOCKET TempSocket = socket(AF_INET,SOCK_DGRAM,0);
	WSAIoctl(TempSocket,SIO_ADDRESS_LIST_QUERY,NULL,0,buffer,16*sizeof(SOCKET_ADDRESS_LIST),&bufferSize,NULL,NULL);
	for (i = 0;i < ((SOCKET_ADDRESS_LIST*)buffer)->iAddressCount;++i)
	{
		addr = ((struct sockaddr_in*)(((SOCKET_ADDRESS_LIST*)buffer)->Address[i].lpSockaddr))->sin_addr;
		printf("%d: %s (%u)\n", i, inet_ntoa(addr), addr);
	}
	closesocket(TempSocket);
#else
	//
	// Posix will also use an Ioctl call to get the IPAddress List
	//
	char szBuffer[16*sizeof(struct ifreq)];
	struct ifconf ifConf;
	struct ifreq ifReq;
	int nResult;
	int LocalSock;
	struct sockaddr_in LocalAddr;
	int tempresults[16];
	int ctr=0;
//	int i;
	/* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
	if ((LocalSock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		DEBUGSTATEMENT(printf("Can't do that\r\n"));
		exit(1);
	}
	/* Get the interface configuration information... */
	ifConf.ifc_len = sizeof szBuffer;
	ifConf.ifc_ifcu.ifcu_buf = (caddr_t)szBuffer;
	nResult = ioctl(LocalSock, SIOCGIFCONF, &ifConf);
	if (nResult < 0)
	{
		DEBUGSTATEMENT(printf("ioctl error\r\n"));
		exit(1);
	}
	/* Cycle through the list of interfaces looking for IP addresses. */
	for (i = 0;(i < ifConf.ifc_len);)
	{
		struct ifreq *pifReq = (struct ifreq *)((caddr_t)ifConf.ifc_req + i);
		i += sizeof *pifReq;
		/* See if this is the sort of interface we want to deal with. */
		strcpy (ifReq.ifr_name, pifReq -> ifr_name);
		if (ioctl (LocalSock, SIOCGIFFLAGS, &ifReq) < 0)
		{
			DEBUGSTATEMENT(printf("Can't get flags\r\n"));
			exit(1);
		}
		/* Skip loopback, point-to-point and down interfaces, */
		/* except don't skip down interfaces */
		/* if we're trying to get a list of configurable interfaces. */
		if ((ifReq.ifr_flags & IFF_LOOPBACK) || (!(ifReq.ifr_flags & IFF_UP)))
		{
			continue;
		}	
		if (pifReq -> ifr_addr.sa_family == AF_INET)
		{
			/* Get a pointer to the address... */
			memcpy (&LocalAddr, &pifReq -> ifr_addr, sizeof pifReq -> ifr_addr);
			if (LocalAddr.sin_addr.s_addr != htonl (INADDR_LOOPBACK))
			{
				tempresults[ctr] = LocalAddr.sin_addr.s_addr;
				printf("%d: %s (%u)\n", ctr, (char*)inet_ntoa(tempresults[ctr]), tempresults[ctr]);
				++ctr;
			}
		}
	}
	close(LocalSock);
//	*pp_int = (int*)malloc(sizeof(int)*(ctr));
//	memcpy(*pp_int,tempresults,sizeof(int)*ctr);
//	return(ctr);
//	if (ctr > 0)
//	   i = ctr - 1;
//	else
	   i = ctr;
#endif
//	printf("IP addresses detected: %d \n", i);
	if (i > 1 )
	{
		printf("Please choose a number for IP address: ");
		while(attemptsLeft > 0)
		{
			fgets(inBuf, 128, stdin);
			if(1 != sscanf(inBuf, "%d", &index))
			{
				printf("Please enter a number: ");
			}
			else 
			if(index < 0 || index >= i)
			{
				printf("Please choose one of the above ");
				attemptsLeft--;
			}
			else
			{	
			#if defined(_POSIX)
				retValue = tempresults[i];
			#else
				addr = ((struct sockaddr_in*)(((SOCKET_ADDRESS_LIST*)buffer)->Address[index].lpSockaddr))->sin_addr;
				retValue = addr.S_un.S_addr;
			#endif				
				break;
			}
		}
	}
	if ((i == 1) || (attemptsLeft == 0))
	{
	#if defined(_POSIX)
		retValue = tempresults[0];
	#else
		addr = ((struct sockaddr_in*)(((SOCKET_ADDRESS_LIST*)buffer)->Address[0].lpSockaddr))->sin_addr;
		retValue = addr.S_un.S_addr;
	#endif
	}
/*
	else if (i == 0 )
	{
		retValue = inet_addr("127.0.0.1");
	}
*/
	return retValue;
}

#define USER	"Venkat"
#define PWD	"password"

int readData(LPSOCKET_INFO pSI)
{
//	DWORD	bytes;
	if (pSI->s == INVALID_SOCKET)
		return -2;
	pSI->wsaBuf.buf = pSI->Buf;
	pSI->wsaBuf.len = BUFFERSIZE;
	pSI->rxb = 0;
	if ((pSI->rxb = recv(pSI->s, pSI->wsaBuf.buf, pSI->wsaBuf.len, 0)) == SOCKET_ERROR)
	{
		printf("receive failed with error\n");
		FreeSocketInformation(pSI->index);
		return -1;
	}
	else
	{
		*(pSI->wsaBuf.buf + pSI->rxb) = 0;
	//	printf("Recd: %s\n", pSI->wsaBuf.buf);
	}

	// If zero bytes are received, this indicates connection is closed.
	if (pSI->rxb == 0)
	{
		FreeSocketInformation(pSI->index);
	}
	return pSI->rxb;
}

int sendData(LPSOCKET_INFO pSI)
{
	if (pSI->s == INVALID_SOCKET)
		return -2;
	if ((pSI->txb = send(pSI->s, pSI->wsaBuf.buf, pSI->wsaBuf.len, 0)) == SOCKET_ERROR)
	{
		printf("send failed with error\n");
		FreeSocketInformation(pSI->index);
		return -1;
	}
	else
	{
		printf("Sent: %s\n", pSI->wsaBuf.buf);
	}
	return pSI->txb;
}
int sendSmallData(LPSOCKET_INFO pSI, char *dta)
{
	char *cp;
	if ((pSI == NULL) || (dta == NULL))
		return 0;
	cp = pSI->wsaBuf.buf = pSI->Buf;
	strcpy(cp, dta);
	pSI->wsaBuf.len = (int)strlen(dta);
	return sendData(pSI);
}
int process_wsend(LPSOCKET_INFO pSI, char *filename)
{
//#define GET_NAMEnSIZE 1
//#define GET_FILE_SIZE 2
//#define GET_FILE_DATA 3

	int result;
//	int state;
	long size;
	long recvSize;
	char *cp;
//	char *filename;
	FILE *fp;
	unsigned long NonBlock;

	if ((pSI == NULL) || (filename == NULL))
		return -1;
	cp = filename + (int) strlen(filename);
	if (cp > filename)
	{
		while (--cp != filename)
		{
			if (*cp == ' ')
				break;
		}
	}
//	if ((cp = strchr(cp, ' ')) != NULL)
	if (cp != filename)
		*cp++ = 0;
	if (cp != NULL)
	{
		if (isdigit(*cp))
			size = atol(cp);
		else
			size = 0;
	}
	else
		size = 0;
	if (size < 1)
	{
		printf("File size not valid (size=%d)\n", (int)size);
		return -1;
	}
	if ((fp=fopen(filename, "wb")) == NULL)
	{
		printf("Could not open file for writing.\n");
		result = -3;
		return result;
	}
        // Put the socket into blocking mode for synchronozed transfer
	NonBlock = 0;
	if (ioctlsocket(pSI->s, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error\n");
		fclose(fp);
		return -1;
	}
	result = sendSmallData(pSI, "SEND");
	recvSize = 0;
	do{
		result = readData(pSI);
		if (result > 0)
		{
			fwrite(pSI->Buf, sizeof(char), result, fp);
			recvSize += result;
			result = sendSmallData(pSI, "OK");
		}
	}while ((recvSize < size) && (result >= 0));
	fclose(fp);
        // Put the socket into non-blocking mode
	NonBlock = 1;
	if (ioctlsocket(pSI->s, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
	   printf("ioctlsocket() failed with error\n");
	   return -1;
	}
	return result;
}
int readSocket(LPSOCKET_INFO pSI)
{
	DWORD Bytes = 0;
	char *cp;
	int result;
//	state dataState;
//	BOOL reply = FALSE;

	result = readData(pSI);
	if (result > 0)
	{
		cp = pSI->wsaBuf.buf;
		switch(currState)
		{
		case GET_USER:
			if (strcmp(pSI->wsaBuf.buf, USER) == 0)
			{
			//	sprintf(pSI->Buf, "PASSWORD: ");
				result = sendSmallData(pSI, "PASSWORD:");
				currState = GET_PWD;
			//	reply = TRUE;
			}
			else
				result = sendSmallData(pSI, "Username does not match\nUSER NAME:");
			break;
		case GET_PWD:
			if (strcmp(pSI->wsaBuf.buf, PWD) == 0)
			{
			//	sprintf(pSI->Buf, "SEND DATA");
				result = sendSmallData(pSI, "GET_DATA");
			//	reply = TRUE;
				currState = READY;
			}
			else
				result = sendSmallData(pSI, "Password does not match\nPASSWORD:");
			break;
		case READY:
			if (strncmp(pSI->wsaBuf.buf, "wsend", strlen("wsend")) == 0)
			{
				result = -1;
				if ((cp=strchr(pSI->Buf, ' ')) != NULL)
				{
					if ( ++cp != NULL)
						result = process_wsend(pSI, cp);
				}
			}
			else
			{
				//printf("Command Recd:%s\n", pSI->Buf);
				result = sendSmallData(pSI, "COMMAND ERROR");
			}
			break;
		default:
			printf("readSocket: Wrong state");
			break;
		}
	}
	return result;
}
#if defined(_POSIX)
DWORD setupServer(void* args)
#else
DWORD WINAPI setupServer(LPVOID args)
#endif
{
	SOCKET ListenSocket;
	SOCKET AcceptSocket;
	struct sockaddr_in InternetAddr;
#ifdef WIN32
	WSADATA wsaData;
#endif
	int Ret;
	fd_set Writer;
	fd_set Reader;
	DWORD i;
	DWORD Total;
	DWORD NonBlock;
//   DWORD Flags;
//   DWORD SendBytes;
//   DWORD RecvBytes;
	struct timeval tv ;

	// Set up the struct timeval for the timeout.
	tv.tv_sec = 0; // 2 seconds
	tv.tv_usec = 300000 ;

#ifdef WIN32
   if ((Ret = WSAStartup(MAKEWORD(2,0),&wsaData)) != 0)
   {
      printf("WSAStartup() failed with error %d\n", Ret);
      WSACleanup();
      return 1;
   }
#endif
   // Create a socket.

 //  if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET) 
   {
      printf("WSASocket() failed with error\n");
      return 1;
   }

   InternetAddr.sin_family = AF_INET;
//   InternetAddr.sin_addr.s_addr = inet_addr("172.16.40.165");//htonl(INADDR_ANY);
//   InternetAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//172.16.40.157");//htonl(INADDR_ANY);
//   InternetAddr.sin_addr.s_addr = inet_addr("192.168.11.2");//172.16.40.157");//htonl(INADDR_ANY);
	InternetAddr.sin_addr.s_addr =  getIPaddress();
	InternetAddr.sin_port = htons(PORT);

   if (bind(ListenSocket, (struct sockaddr *) &InternetAddr, sizeof(InternetAddr))
      == SOCKET_ERROR)
   {
      printf("Binding failed with error\n");
      return 1;
   }

   if (listen(ListenSocket, 5))
   {
      printf("listen failed with error\n");
      return 1;
   }

   // Change the socket mode on the listening socket from blocking to non-block 

   NonBlock = 1;
   if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
   {
      printf("ioctlsocket() failed \n");
      return 1;
   }

	printf("\nServer started.\n");
   while(1)
   {
		if (_kbhit())
		{
			if (_getch() == 'q')
				break;
		}
		//	_getch();
      // Initialize the Read and Write socket set.
      FD_ZERO(&Reader);
      FD_ZERO(&Writer);

      // Check for connection attempts.

      FD_SET(ListenSocket, &Reader);

      // Set Read and Write notification for each socket based on the
      // current state the buffer.  

      for (i = 0; i < TotalSockets; i++)
      //   if (SocketList[i]->rxb > SocketList[i]->txb)
      //      FD_SET(SocketList[i]->s, &Writer);
      //   else
            FD_SET(SocketList[i]->s, &Reader);

//      if ((Total = select(0, &Reader, &Writer, NULL, NULL)) == SOCKET_ERROR)
      if ((Total = select(FD_SETSIZE, &Reader, NULL, NULL, &tv)) == SOCKET_ERROR)
      {
         printf("select function returned with error\n");
         return 1;
      }
    if (Total > 0)
    {
      printf("Total: %d \n", (int)Total);
      // Check for arriving connections on the listening socket.
      if (FD_ISSET(ListenSocket, &Reader))
      {
         Total--;
         if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
         {

            // Set the accepted socket to non-blocking mode so the server will
            // not get caught in a blocked condition on WSASends

            NonBlock = 1;
            if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
            {
               printf("ioctlsocket() failed with error\n");
               return 1;
            }

            if (CreateSocketInformation(AcceptSocket) == FALSE)
               return 1;

         }
         else
         {		
         //   if (WSAGetLastError() != WSAEWOULDBLOCK)
         //   {
               printf("accept() failed with error\n");
               return 1;
           // }
         }
      }
     }
      // Check each socket for Read and Write notification for Total number of sockets

      for (i = 0; Total > 0 && i < TotalSockets; i++)
      {
         LPSOCKET_INFO SocketInfo = SocketList[i];

         // If the Reader is marked for this socket then this means data
         // is available to be read on the socket.

         if (FD_ISSET(SocketInfo->s, &Reader))
         {
            Total--;

            SocketInfo->wsaBuf.buf = SocketInfo->Buf;
            SocketInfo->wsaBuf.len = BUFFERSIZE;

      //      Flags = 0;
			readSocket(SocketInfo);
			printf("\n>>");
          }

        }
     }
     for (i = 0; i < TotalSockets; i++)
     {
          FreeSocketInformation(i);
     }
#if defined (_POSIX)
   pthread_exit((void *) 1);
#endif
    return 1;
}
#if defined(_POSIX)
int main(int argc, char* argv[])
{
   pthread_t thread;
   pthread_attr_t attr;
   int rc;
   void *status;

   /* Initialize and set thread detached attribute */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      printf("Creating thread\n");
      rc = pthread_create(&thread, &attr, (void*)&setupServer, (void*)NULL); 
      if (rc)
      {
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }

   /* Free attribute and wait for the other threads */
   pthread_attr_destroy(&attr);
      rc = pthread_join(thread, &status);
      if (rc)
      {
         printf("ERROR; return code from pthread_join() is %d\n", rc);
         exit(-1);
      }
      printf("Completed join with thread status= %ld\n", (long)status);

   pthread_exit((void *) 0);
   return 1;
}
#else
int _tmain(int argc, _TCHAR* argv[])
{
	
	HANDLE	stHandle;
	DWORD code;
//	serverStatus = 1;
	stHandle = (HANDLE)CreateThread(NULL,0,&setupServer,NULL,0,0);
	GetExitCodeThread(stHandle, &code);
//	printf("Thread status: %d\r\n", code);
	WaitForSingleObject(stHandle, INFINITE);
	GetExitCodeThread(stHandle, &code);
//	printf("Thread status: %d\r\n", code);
	return 1;
}
#endif
BOOL CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFO SI;
	  
	printf("Accepted socket\n");

#ifdef WIN32
	if ((SI = (LPSOCKET_INFO) GlobalAlloc(GPTR,
	  sizeof(SOCKET_INFO))) == NULL)
#else
	if ((SI = (LPSOCKET_INFO) malloc(sizeof(SOCKET_INFO))) == NULL)
#endif
	{
		printf("Memory allocation failed\n");
		return FALSE;
	}

	// Prepare SocketInfo structure for use.

	SI->s = s;
	SI->txb = 0;
	SI->rxb = 0;
	SI->index = TotalSockets;

	SocketList[TotalSockets] = SI;

	currState = GET_USER;
	sprintf(SI->Buf, "USER NAME:");
	SI->wsaBuf.buf = SI->Buf;
	SI->wsaBuf.len = (int)strlen(SI->Buf);
	sendData(SI);
	TotalSockets++;
	return(TRUE);
}

void FreeSocketInformation(DWORD Index)
{
   LPSOCKET_INFO SI = SocketList[Index];
   DWORD i;

   printf("Closing socket\n");


#ifdef WIN32
   closesocket(SI->s);
   GlobalFree(SI);
#else
   close(SI->s);
   free(SI);
#endif
   // Remove from the socket array
   for (i = Index; i < TotalSockets; i++)
   {
      SocketList[i] = SocketList[i + 1];
   }

   TotalSockets--;
}


