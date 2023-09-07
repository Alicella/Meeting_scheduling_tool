#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include "helper_funcs.h"

#define B_UDP_PORT "22524"
#define M_UDP_PORT 23524
#define IP_addr "127.0.0.1"
#define MAXBUFLEN 1000

int main()
{
   // read input files by lines
   // each line is stored as an entry of hash table
   // usernames are stored in a vector

   std::vector<std::string> usernames;
   std::unordered_map<std::string, std::vector<std::vector<int>>> userSlots;

   std::ifstream file("b.txt");
   std::string line;

   while (getline(file, line))
   {
      if (line.empty())
      {
         continue;
      }
      std::istringstream iss(line);

      // store string before ';' in usernames
      std::string user;
      getline(iss, user, ';');
      // removing white space, refering to: https://stackoverflow.com/questions/83439/remove-spaces-from-stdstd::string-in-c
      //  why using ::isspace but can't use std::isspace???

      user.erase(remove_if(user.begin(), user.end(), ::isspace), user.end());

      usernames.push_back(user);

      // store std::string after ';'
      std::string slotStr;
      getline(iss, slotStr);

      std::vector<std::vector<int>> slots = slotStr_to_vector(slotStr);

      userSlots.insert(make_pair(user, slots));
   }

   file.close();

   // creating UDP socket
   // code to set up the UDP socket refers to Beej's guide

   int sockfd;
   struct addrinfo hints, *servinfo, *p;
   int rv;
   int yes = 1;
   int numbytes;
   struct sockaddr_storage their_addr;
   char buf[MAXBUFLEN];
   socklen_t addr_len;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_DGRAM;

   if ((rv = getaddrinfo(IP_addr, B_UDP_PORT, &hints, &servinfo)) != 0)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and make a socket
   for (p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
                           p->ai_protocol)) == -1)
      {
         perror("talker: socket");
         continue;
      }
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
      {
         perror("setsockopt");
         exit(1);
      }
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
      {
         close(sockfd);
         perror("server: bind");
         continue;
      }

      break;
   }

   if (p == NULL)
   {
      fprintf(stderr, "talker: failed to create socket\n");
      return 2;
   }

   freeaddrinfo(servinfo);

   printf("Server B is up and running using UDP on port %s.\n", B_UDP_PORT);

   // put username into msg
   std::stringstream ss;
   ss << "B" << ' '; // add an identifier in the front of the msg

   for (std::string s : usernames)
   {
      ss << s << ' ';
   }
   std::string tosend = ss.str();
   const char *msg = tosend.c_str();

   // told by TA in OH to define M's address this way
   struct sockaddr_in M_addr;
   M_addr.sin_family = AF_INET;
   inet_pton(AF_INET, IP_addr, &M_addr.sin_addr.s_addr);
   M_addr.sin_port = htons(M_UDP_PORT);

   if ((numbytes = sendto(sockfd, msg, strlen(msg), 0, (sockaddr *)&M_addr, sizeof(M_addr))) == -1)
   {
      perror("talker: sendto");
      exit(1);
   }

   std::cout << "Server B finished sending a list of usernames to Main Server." << std::endl;

   // keep processing requests from the main server
   while (1)
   {
      addr_len = sizeof their_addr;

      if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
      {
         perror("recvfrom");
         exit(1);
      }
      buf[numbytes] = '\0';

      printf("Server B received the usernames from Main Server using UDP over port %s\n", B_UDP_PORT);

      std::vector<std::string> request;
      std::stringstream ss_r(buf);
      std::string name;

      while (getline(ss_r, name, ','))
      {
         name.erase(remove_if(name.begin(), name.end(), ::isspace), name.end());
         request.push_back(name);
      }

      std::vector<std::vector<int>> cur = userSlots[request[0]];

      for (int i = 1; i < request.size(); i++)
      {
         cur = find_intersection(cur, userSlots[request[i]]);

         if (cur.size() == 0)
         {
            break;
         }
      }

      // send the intersection cur to serverM

      std::string result = slotVector_to_str(cur);
      printf("Found the intersection result: [%s] for %s\n", result.c_str(), buf);

      result.push_back('B');

      const char *msg = result.c_str();

      if ((numbytes = sendto(sockfd, msg, strlen(msg), 0, (sockaddr *)&M_addr, sizeof(M_addr))) == -1)
      {
         perror("talker: sendto");
         exit(1);
      }

      std::cout << "Server B finished sending the response to Main Server." << std::endl;
   }

   close(sockfd);
}