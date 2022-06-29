/**
 * @brief Server.cpp
 * The Server class is a server that accepts a socket connection to 
 * a Client and performs reads from the writes the client sends over. It 
 * then keeps track of the number of reads performed and writes it back to the 
 * Client.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <pthread.h>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

// pthread function to read and write to and from client
void *acceptSocket(void *arg)
{

   // allocate buffer
   char databuf[BUFFSIZE];
   // zero it out
   bzero(databuf, BUFFSIZE);

   // assign newSD
   int newSD = *(int *)arg;

   // read in number of iterations to perform from client
   int iterations;
   read(newSD, &iterations, 4);
   iterations = ntohl(iterations);
   cout << "Printing number of iterations " << iterations << endl;

   // number of reads is total number of reads performed
   int numberOfReads = 0;
   int totalR = 0;

   // while total number of bytes read is less than what should be read
  for(int i = 0; i<iterations; i++){

      // number of bytes read in total
      int totalBytesRead = 0;

      while (totalBytesRead != BUFFSIZE)
      {
         // read
         int bytesRead = read(newSD, databuf, (BUFFSIZE - totalBytesRead));
         totalBytesRead += bytesRead;
         totalR += bytesRead;
         numberOfReads++;
      
      }
   }


   // send number of reads performed back to client
   int tmp = htonl((uint32_t)numberOfReads);
   write(newSD, &tmp, sizeof(tmp));

   close(newSD);

   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

   int port;         // port number
   char *serverName; // server name

   // error message for incorrect input
   if (argc != 2)
   {
      cerr << "Usage: " << argv[0] << "serverName" << endl;
      return -1;
   }

   // set argument
   port = stoi(argv[1]);

   // build address, create accept socket address
   sockaddr_in acceptSocketAddress;
   bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
   // use internet
   acceptSocketAddress.sin_family = AF_INET;
   // server should listen for any IP address trying to connect
   acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   // set port address
   acceptSocketAddress.sin_port = htons(port);

   // create and open socket
   int serverSD = socket(AF_INET, SOCK_STREAM, 0);
   const int on = 1;
   setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
   cout << "Socket #: " << serverSD << endl;

   // bind socket to socket address created
   bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));

   // listen
   listen(serverSD, NUM_CONNECTIONS); // setting number of pending connections

   // returned a socket when client connects (index into file descriptor)
   sockaddr_in newSockAddr;
   socklen_t newSockAddrSize = sizeof(newSockAddr);

   while (true)
   {
      // accept
      //  use this socket to read and write from (newSD)
      int newSD = accept(serverSD, (sockaddr *)&newSockAddr, &newSockAddrSize);
      cout << "Accepted Socket #: " << newSD << endl;

      // create pthread and pass in newSD
      pthread_t socketThread;
      cout << "Creating pthread" << endl;
      pthread_create(&socketThread, NULL, acceptSocket, (void *)&newSD);
   }

   close(serverSD);

   return 0;
}
