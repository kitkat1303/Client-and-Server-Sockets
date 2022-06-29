/**
 * @brief Client.cpp
 * The Client class is a creates a socket connection the the Server class
 * and writes a certain amount of bytes to the server in different ways based 
 * on test cases entered. It does so a certain amount of iterations and then 
 * tracks the amount of time it takes to write to the server and for the server 
 * to perform reads on the data. It then outputs how long it took and the 
 * throughput of the operations.
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
using namespace std;

const int BUFFSIZE = 1500;

int main(int argc, char *argv[])
{
   // 6 arguments
   char *serverName;   // server name
   char *port;         // port number
   int iterations = 0; // number of iterations to perform
   int nbufs = 0;      // number of elements
   int bufsize = 0;    // bytes per element
   int type = -1;      // test type to perform
   double time = 0;    // time it takes to perform the test
   int numReads = 0;

   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int clientSD = -1;
   

   // error message for incorrect input
   if (argc != 7)
   {
      cerr << "Usage: " << argv[0] << "serverName" << endl;
      return -1;
   }

   // set arguments
   serverName = argv[1];
   port = argv[2];
   iterations = stoi(argv[3]); // number of iterations
   nbufs = stoi(argv[4]);
   bufsize = stoi(argv[5]);
   type = stoi(argv[6]);

   char** databuf = NULL; 

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6*/
   hints.ai_socktype = SOCK_STREAM; /* TCP */
   hints.ai_flags = 0;              /* Optional Options*/
   hints.ai_protocol = 0;           /* Allow any protocol*/
   int rc = getaddrinfo(serverName, port, &hints, &result);
   if (rc != 0)
   {
      cerr << "ERROR: " << gai_strerror(rc) << endl;
      exit(EXIT_FAILURE);
   }

   // iterate through addresses to connect
   for (rp = result; rp != NULL; rp = rp->ai_next)
   {
      clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (clientSD == -1)
      {
         continue;
      }
      // socket has been connected
      rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
      if (rc < 0)
      {
         cerr << "Connection Failed" << endl;
         close(clientSD);
         return -1;
      }
      else // success
      {
         break;
      }
   }

   // error for no valid address
   if (rp == NULL)
   {
      cerr << "No valid address" << endl;
      exit(EXIT_FAILURE);
   }
   else
   {
      cout << "Client Socket: " << clientSD << endl;
   }
   freeaddrinfo(result);

   // send number of iterations to server
   int tmp = htonl((uint32_t)iterations);
   write(clientSD, &tmp, sizeof(tmp));

   // fill databuff
   databuf = new char* [nbufs];
   for (int i = 0; i < nbufs; i++)
   {
      databuf[i] = new char [bufsize];
      for (int j = 0; j < bufsize; j++)
      {
         databuf[i][j] = 'z';
      }
   }

   if (type == 1)
   {
      auto start = std::chrono::high_resolution_clock::now();
      int totalW = 0;

      // for total number of iterations
      for (int i = 0; i < iterations; i++)
      {
         for (int j = 0; j < nbufs; j++)
         {
            int byteWritten = write(clientSD, databuf[j], bufsize);
            totalW += byteWritten;
         }
      }
      // get time
      cout << "Total written at end: " << totalW << endl;

      // get number of reads done by server
      numReads = 0;
      read(clientSD, &numReads, 4);
      numReads = ntohl(numReads);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      time = diff.count();
   }
   else if (type == 2)
   {

      struct iovec vector[nbufs];

      auto start = std::chrono::high_resolution_clock::now();

      int totalWritten = 0;
      // for number of iterations
      for (int i = 0; i < iterations; i++)
      {
         for (int j = 0; j < nbufs; j++)
         {
            vector[j].iov_base = databuf[j];
            vector[j].iov_len = bufsize;
         }
         totalWritten += writev(clientSD, vector, nbufs);
      }

      // get number of reads done by server
      numReads = 0;
      read(clientSD, &numReads, 4);
      numReads = ntohl(numReads);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      time = diff.count();

   }
   else if (type == 3)
   {
      auto start = std::chrono::high_resolution_clock::now();
      int totalWritten = 0;

      // for total number of iterations
      for (int i = 0; i < iterations; i++)
      {
         totalWritten += write(clientSD, databuf, BUFFSIZE);
      }

      // get number of reads done by server
      numReads = 0;
      read(clientSD, &numReads, 4);
      numReads = ntohl(numReads);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      time = diff.count();
      
   }
   else
   {
      cerr << "Incorrect test case entered, please enter 1-3." << endl;
   }

   

   double throughput = 0;

   if (time != 0)
   {
      throughput = (numReads * BUFFSIZE) / time;
   }

   // print
   cout << "Test " + to_string(type) + " number of iterations " +
               to_string(iterations) + " time = " + to_string(time) +
               " #reads " + to_string(numReads) + " throughput " +
               to_string(throughput) + "Gbps."
        << endl;

   close(clientSD);
   return 0;
}