#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>

#define M_TCP_PORT "24524"
#define IP_addr "127.0.0.1"
#define MAXDATASIZE 1000
#define NAMENOTEXIST '0'
#define CLOSE_REQUEST '1'
#define MSG_END '3'

using namespace std;

// the code that sets up the TCP socket refers to Beej's guide
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in *)sa)->sin_addr);
   }
   return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main()
{
   int sockfd, numbytes;
   char buf[MAXDATASIZE];
   struct addrinfo hints, *servinfo, *p;
   int rv;
   char s[INET6_ADDRSTRLEN];

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;

   if ((rv = getaddrinfo(IP_addr, M_TCP_PORT, &hints, &servinfo)) != 0)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }
   // loop through all the results and connect to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1)
      {
         perror("client: socket");
         continue;
      }

      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
      {
         close(sockfd);
         perror("client: connect");
         continue;
      }
      break;
   }

   if (p == NULL)
   {
      fprintf(stderr, "client: failed to connect\n");
      return 2;
   }

   cout << "Client is up and running." << endl;

   freeaddrinfo(servinfo); // all done with this structure

   while (1)
   {
      // Taking user inputs
      string request;
      while (request.empty())
      {
         cout << "Please enter the usernames to check schedule availability: " << endl;
         getline(cin, request);
      }

      // getline(cin, request);

      const char *msg = request.c_str();
      int len = strlen(msg);
      if (send(sockfd, msg, len, 0) == -1)
      {
         perror("send");
      }

      cout << "Client finished sending the usernames to Main Server." << endl;

      struct sockaddr_in sockName;
      socklen_t sockLen = sizeof sockName;
      if (getsockname(sockfd, (struct sockaddr *)&sockName, &sockLen) == 1)
      {
         perror("getsockname");
         exit(1);
      }

      while (1)
      {
         string buffer;
         while (1)
         {
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
            {
               perror("recv");
               exit(1);
            }

            buf[numbytes] = '\0';
            buffer += buf;

            if (buf[numbytes - 1] == MSG_END)
            {
               buf[numbytes - 1] = '\0';
               break;
            }
         }

         // cout << "buffer: " << buffer << endl;

         istringstream iss(buffer);
         vector<string> msgs;
         string single_msg;

         while (getline(iss, single_msg, '\n'))
         {
            msgs.push_back(single_msg);
         }

         char *msg1 = const_cast<char *>(msgs[0].c_str());
         char *msg2 = const_cast<char *>(msgs[1].c_str());
         // cout << "msg1: " << msg1 << endl;
         // cout << "msg2: " << msg2 << endl;

         if (msg1[strlen(msg1) - 1] == NAMENOTEXIST)

         {
            msg1[strlen(msg1) - 1] = '\0';
            printf("Client received the reply from Main Server using TCP over port %hu: %s do not exist.\n", ntohs(sockName.sin_port), msg1);

            // none of the input names exists, need to close this request
            if (msg2[strlen(msg2) - 1] == CLOSE_REQUEST)
            {
               break;
            }
         }

         else
         {
            printf("Client received the reply from Main Server using TCP over port %hu: Time intervals [%s] works for %s.\n",
                   ntohs(sockName.sin_port), msg2, msg1);
            std::cout << std::endl;
            break;
         }
      }
   }

   close(sockfd);

   return 0;
}