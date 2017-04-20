//==================================================== file = tcpServer.c =====
//=  A message "server" program to demonstrate sockets programming            =
//=============================================================================
//=  Notes:                                                                   =
//=    1) This program conditionally compiles for Winsock and BSD sockets.    =
//=       Set the initial #define to WIN or BSD as appropriate.               =
//=    2) This program serves a message to program tcpClient running on       =
//=       another host.                                                       =
//=    3) The steps #'s correspond to lecture topics.                         =
//=---------------------------------------------------------------------------=
//=  Example execution: (tcpServer and tcpClient running on host 127.0.0.1)   =
//=    Waiting for accept() to complete...                                    =
//=    Accept completed (IP address of client = 127.0.0.1  port = 49981)      =
//=    Received from client: This is a reply message from CLIENT to SERVER    =
//=---------------------------------------------------------------------------=
//=  Build: bcc32 tcpServer.c or cl tcpServer.c wsock32.lib for Winsock       =
//=         gcc tcpServer.c -lnsl for BSD                                     =
//=---------------------------------------------------------------------------=
//=  Execute: tcpServer                                                       =
//=---------------------------------------------------------------------------=
//=  Author: Ken Christensen                                                  =
//=          University of South Florida                                      =
//=          WWW: http://www.csee.usf.edu/~christen                           =
//=          Email: christen@csee.usf.edu                                     =
//=---------------------------------------------------------------------------=
//=  History:  KJC (08/02/08) - Genesis (from server.c)                       =
//=            KJC (09/07/09) - Minor clean-up                                =
//=            KJC (09/22/13) - Minor clean-up to fix warnings                =
//=============================================================================
#define  BSD                // WIN for Winsock and BSD for BSD sockets


//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()

#include <iostream>
#ifdef WIN
  #include <windows.h>      // Needed for all Winsock stuff
#endif
#ifdef BSD
  #include <unistd.h>
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
	//#include <readline/readline.h>
	//#include <readline/history.h>
#endif
using namespace std;
//----- Defines ---------------------------------------------------------------
#define  PORT_NUM   1052    // Arbitrary port number for the server

//definitions
#define PITCH_MIN						-180.0000000
#define PITCH_MAX							180.0000000
#define ROLL_MIN						-180.0000000
#define ROLL_MAX						180.0000000
#define AIRSPEED_MIN				0.0000000
#define AIRSPEED_MAX				400.0000000
#define HEADING_MIN					0.0000000
#define HEADING_MAX					360.0000000
#define SLIPSKID_MIN				-1.5000000
#define SLIPSKID_MAX				1.5000000
#define LOCALIZERSCALE_MIN	-2.0000000
#define LOCALIZERSCALE_MAX	2.0000000
#define GLIDESLOPE_MIN			-2.0000000
#define GLIDESLOPE_MAX			2.0000000
#define ALTITUDE_MIN				0
#define ALTITUDE_MAX				20000

//===== Data Types ============================================================
struct flightData{
	float pitch, roll, airspeed, heading, slipSkid, localizerScale, glideSlope;
	int altitude;
	};

//===== Functions =============================================================

//-------------------	bool checkFlightDataValues()	---------------------------------
int checkFlightDataValues(struct flightData data){
		if(PITCH_MIN <= data.pitch && data.pitch <= PITCH_MAX)
      if(ROLL_MIN <= data.roll && data.roll<= ROLL_MAX)
        if(AIRSPEED_MIN <= data.airspeed && data.roll<= AIRSPEED_MAX)
          if(HEADING_MIN <= data.heading && data.roll <= HEADING_MAX)
            if(SLIPSKID_MIN<= data.slipSkid && data.roll<=SLIPSKID_MAX )
              if(LOCALIZERSCALE_MIN<= data.localizerScale && data.roll <= LOCALIZERSCALE_MAX)
                if(GLIDESLOPE_MIN <= data.glideSlope && data.roll <=GLIDESLOPE_MAX )
                  if(ALTITUDE_MIN <= data.altitude && data.roll<= ALTITUDE_MAX)
                    return 0;
		else{
      printf( " %f  %f\n", data.pitch , data.roll);
			printf("Error: checkFlightDataValues -> packet values out of range\n");
			return 1;
			}
}

//-------------------	void sendPacket()	---------------------------------
void sendPacket(struct flightData, char* temp, int retCode, int connect_s){

	retCode = send(connect_s, temp, (strlen(temp) +1), 0);
	if (retCode < 0){
		printf("*** ERROR - send() failed \n");
		exit(-1);
	}
	// >>> Step #6 <<<
	// Receive from the client using the connect socket
	retCode = recv(connect_s, temp, sizeof(temp), 0);
	if (retCode < 0){
	printf("*** ERROR - recv() failed \n");
	exit(-1);
	}
}

//-------------------	void flightDataToCSV()	---------------------------------
void flightDataToCSV(struct flightData data, char* temp){
	sprintf(temp, "%f,%f,%f,%f,%f,%f,%f,%i ", data.pitch, data.roll, data.airspeed, data.heading, data.slipSkid, data.localizerScale, data.glideSlope, data.altitude);
printf("temp is %s\n", temp);
}


//-------------------	sendFlightData ()	---------------------------------
//3)sendpacket
void sendFlightData (struct flightData data, int retCode, int connect_s ){
	//1) checkData

//  printf( " %f  %f\n", data.pitch , data.roll);
	if (checkFlightDataValues(data) == 1){
    return;
  }

	//2) format flightData
	char temp[4096];
	char* temp_ptr = temp;
	//take flight data struct and place it into temp[]
	flightDataToCSV(data, temp_ptr);


	//3)sendpacket
	retCode = send(connect_s, temp, (strlen(temp) +1), 0);
	if (retCode < 0){
	  printf("*** ERROR - send() failed \n");
	  exit(-1);
	}
	// >>> Step #6 <<<
	// Receive from the client using the connect socket
	retCode = recv(connect_s, temp, sizeof(temp), 0);
	if (retCode < 0){
		printf("*** ERROR - recv() failed \n");
		exit(-1);
	}

}



//===== Main program ==========================================================

int main()
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff

 WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  welcome_s;       // Welcome socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  connect_s;       // Connection socket descriptor
  struct sockaddr_in   client_addr;     // Client Internet address
  struct in_addr       client_ip_addr;  // Client IP address
  socklen_t            addr_len;        // Internet address length
  char                 out_buf[4096];   // Output buffer for data
  char                 in_buf[4096];    // Input buffer for data
  int                  retcode;         // Return code
  char 				 				 messageToClient[4096];
  char 								*messagePtr;
  char 								*in_buf_ptr;
  char 								*out_buf_ptr;
  int 								*retCode_ptr;
        retCode_ptr= &retcode;
  messagePtr = messageToClient;
#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif


	//*******************Create Server*******************************************
  // >>> Step #1 <<<
  // Create a welcome socket
  //   - AF_INET is Address Family Internet and SOCK_STREAM is streams
  welcome_s = socket(AF_INET, SOCK_STREAM, 0);
  if (welcome_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }

  // >>> Step #2 <<<
  // Fill-in server (my) address information and bind the welcome socket
  server_addr.sin_family = AF_INET;                 // Address family to use
  server_addr.sin_port = htons(PORT_NUM);           // Port number to use
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address

retcode = bind(welcome_s, (struct sockaddr *)&server_addr,
    sizeof(server_addr));
  if (retcode < 0)
  {
    printf("*** ERROR - bind() failed \n");
    exit(-1);
  }

  // >>> Step #3 <<<
  // Listen on welcome socket for a connection
  listen(welcome_s, 1);

  // >>> Step #4 <<<
  // Accept a connection.  The accept() will block and then return with
  // connect_s assigned and client_addr filled-in.
  printf("Waiting for accept() to complete... \n");
  addr_len = sizeof(client_addr);
  connect_s = accept(welcome_s, (struct sockaddr *)&client_addr, &addr_len);
  if (connect_s < 0)
  {
    printf("*** ERROR - accept() failed \n");
    exit(-1);
  }

  // Copy the four-byte client IP address into an IP address structure
  memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);

  // Print an informational message that accept completed
  printf("Accept completed (IP address of client = %s  port = %d) \n",
    inet_ntoa(client_ip_addr), ntohs(client_addr.sin_port));

  // >>> Step #5 <<<
  // Send to the client using the connect socket
  strcpy(out_buf, "This is a message from SERVER to CLIENT");
  retcode = send(connect_s, out_buf, (strlen(out_buf) + 1), 0);




if (retcode < 0)
  {
    printf("*** ERROR - send() failed \n");
    exit(-1);
  }

  // >>> Step #6 <<<
  // Receive from the client using the connect socket
  retcode = recv(connect_s, in_buf, sizeof(in_buf), 0);
  if (retcode < 0)
  {
    printf("*** ERROR - recv() failed \n");
    exit(-1);
  }
//*********************End of server setup*****************************************
//*********************Send all data for the client*******************************

	printf("\n");
	//int exitCondition = 0;
	//while(!exitCondition){
	//	exitCondition = interface();
	//	sendFlightData(data1, retcode, connect_s);
	//}

	//return 0;
	string x;
	struct flightData data1;

	data1.pitch						=0.0;
	data1.roll						=0.0;
	data1.airspeed				=0.0;
	data1.heading 				=0.0;
	data1.slipSkid 				=0.0;
	data1.localizerScale  =0.0;
	data1.glideSlope 			=0.0;
	data1.altitude 				= 0;
	sendFlightData(data1, retcode, connect_s);

	printf("Pitch Test: Press Enter to contine.\n");
	cin >> x;
	//pitch test
	for(int i = 0; i < 720; i++){
		sendFlightData(data1, retcode, connect_s);
			if (i < 360){
				data1.pitch+=0.5;
        data1.roll+=0.5;
        data1.airspeed+=1;
        data1.heading+=1;
        data1.altitude+=15;

      }
			else{
				data1.pitch-=0.5;
        data1.roll-=0.5;
        data1.airspeed-=1;
      }//usleep(1);
usleep(1000);
}

	printf("Heading Test: Press Enter to contine.\n");
	cin >> x;
	//heading test

	printf("Slipball Test: Press Enter to contine.\n");

	//slipSkid test
	for(int i = 0; i < 40; i++){

      sendFlightData(data1, retcode, connect_s);
      if (i < 20){
				data1.slipSkid+=0.1;
        data1.localizerScale+=0.1;
  				data1.glideSlope+=0.1;
      }
			else{
				data1.slipSkid-=0.1;
        data1.localizerScale-=0.1;
        data1.glideSlope-=0.1;
      }
      usleep(250000);
}

//*********************end Send all data for the client************
//********************close server*********************************
  // >>> Step #7 <<<
  // Close the welcome and connect sockets
#ifdef WIN
  retcode = closesocket(welcome_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
  retcode = closesocket(connect_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }

#endif
#ifdef BSD
  retcode = close(welcome_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
  retcode = close(connect_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
#endif

#ifdef WIN
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero and terminate
  return(0);
}
