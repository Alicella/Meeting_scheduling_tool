#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include "helper_funcs.h"

#define M_TCP_PORT "24524"
#define M_UDP_PORT "23524"
#define A_UDP_PORT 21524
#define B_UDP_PORT 22524
#define IP_addr "127.0.0.1"

#define BACKLOG 10
#define MAXBUFLEN 5000
#define MAXDATASIZE 1000
#define NAMENOTEXIST "0\n"
#define CLOSE_REQUEST "1\n"
#define CONTINUE_REQUEST "2\n"
#define MSG_END '3'

// the code that sets up TCP and UDP sockets refered to Beej's guide
void sigchld_handler(int s)
{
   // waitpid() might overwrite errno, so we save and restore it:
   int saved_errno = errno;
   while (waitpid(-1, NULL, WNOHANG) > 0)
      ;
   errno = saved_errno;
}

// get sockaddr IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET)
   {
      return &(((struct sockaddr_in *)sa)->sin_addr);
   }

   return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{

   // Seting up UDP socket
   int sockfd_u;
   struct addrinfo hints_u, *servinfo_u, *p_u;
   int rv_u;
   int numbytes_u;
   struct sockaddr_storage their_addr_u;
   char buf_u[MAXBUFLEN];
   socklen_t addr_len_u;

   memset(&hints_u, 0, sizeof hints_u);
   hints_u.ai_family = AF_INET;
   hints_u.ai_socktype = SOCK_DGRAM;
   hints_u.ai_flags = AI_PASSIVE; // use my IP

   if ((rv_u = getaddrinfo(IP_addr, M_UDP_PORT, &hints_u, &servinfo_u)) != 0)
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_u));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for (p_u = servinfo_u; p_u != NULL; p_u = p_u->ai_next)
   {
      if ((sockfd_u = socket(p_u->ai_family, p_u->ai_socktype,
                             p_u->ai_protocol)) == -1)
      {
         perror("listener: socket");
         continue;
      }
      if (bind(sockfd_u, p_u->ai_addr, p_u->ai_addrlen) == -1)
      {
         close(sockfd_u);
         perror("listener: bind");
         continue;
      }
      break;
   }

   if (p_u == NULL)
   {
      fprintf(stderr, "listener: failed to bind socket\n");
      return 2;
   }

   freeaddrinfo(servinfo_u);

   printf("Main Server is up and running.\n");

   int recvd_server = 0;
   std::vector<std::string> usernamesA;
   std::vector<std::string> usernamesB;

   while (1)
   {
      addr_len_u = sizeof their_addr_u;

      if ((numbytes_u = recvfrom(sockfd_u, buf_u, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *)&their_addr_u, &addr_len_u)) == -1)
      {
         perror("recvfrom");
         exit(1);
      }
      buf_u[numbytes_u] = '\0';

      std::stringstream ss_u(buf_u);
      std::string name_u;
      std::getline(ss_u, name_u, ' ');

      if (buf_u[0] == 'A')
      {
         while (getline(ss_u, name_u, ' '))
         {
            usernamesA.push_back(name_u);
         }

         recvd_server++;
         printf("Main Server received the username list from server A using UDP over port %s\n", M_UDP_PORT);
         // Test what msg serverM recieved
         // printf("listener: packet contains \"%s\"\n", buf);
         // for (std::string name : usernamesA)
         // {
         //    std::cout << name << std::endl;
         // }
      }
      else if (buf_u[0] == 'B')
      {
         while (getline(ss_u, name_u, ' '))
         {
            usernamesB.push_back(name_u);
         }
         recvd_server++;
         printf("Main Server received the username list from server B using UDP over port %s\n", M_UDP_PORT);
      }

      if (recvd_server == 2) // serverM has recieved username list from both back end servers
      {
         break;
      }
   }

   // printf("listener: got packet from %s\n",
   //        inet_ntop(their_addr_u.ss_family,
   //                  get_in_addr((struct sockaddr *)&their_addr_u),
   //                  s, sizeof s));
   // printf("listener: packet is %d bytes long\n", numbytes_u);

   // setting up TCP server socket
   int sockfd, new_fd;
   int numbytes;
   struct addrinfo hints, *servinfo, *p;
   struct sockaddr_storage their_addr;
   socklen_t sin_size;
   struct sigaction sa;
   int yes = 1;
   char s[INET6_ADDRSTRLEN];
   int rv;
   char buf[MAXDATASIZE];

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;

   if ((rv = getaddrinfo(NULL, M_TCP_PORT, &hints, &servinfo)) != 0) // why using NULL: p22
   {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   // loop through all the results and bind to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next)
   {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
      {
         perror("server: socket");
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

   freeaddrinfo(servinfo); // all done with this structure

   if (p == NULL)
   {
      fprintf(stderr, "server: failed to bind\n");
      exit(1);
   }

   if (listen(sockfd, BACKLOG) == -1)
   {
      perror("listen");
      exit(1);
   }

   sa.sa_handler = sigchld_handler; // reap all dead processes
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGCHLD, &sa, NULL) == -1)
   {
      perror("sigaction");
      exit(1);
   }

   // main accept() loop
   while (1)
   {
      sin_size = sizeof their_addr;
      new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
      if (new_fd == -1)
      {
         perror("accept");
         continue;
      }

      // inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));

      // printf("server: got connection from %s\n", s);
      printf("Main Server received the request from client using TCP over port %s\n", M_TCP_PORT);

      // keep receiving requests from client
      while (1)
      {
         if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
         {
            perror("recv");
            exit(1);
         }
         buf[numbytes] = '\0';

         // printf("client: received '%s'\n", buf);
         std::vector<std::string> requestA;
         std::vector<std::string> requestB;
         std::vector<std::string> notExist;
         std::string requestAstr;
         std::string requestBstr;
         std::string requestABstr;

         std::stringstream ss_t(buf);
         std::string name;

         while (std::getline(ss_t, name, ' '))
         {
            if (std::find(usernamesA.begin(), usernamesA.end(), name) != usernamesA.end())
            {
               requestA.push_back(name);
            }
            else if (std::find(usernamesB.begin(), usernamesB.end(), name) != usernamesB.end())
            {
               requestB.push_back(name);
            }
            else
            {
               notExist.push_back(name);
            }
         }

         std::stringstream ss;

         // send the names that don't exist to client
         if (notExist.size() > 0)
         {
            std::string notExistNames = namesVector_to_str(notExist);
            ss << notExistNames << NAMENOTEXIST; // 0
            std::string tosend = ss.str();
            const char *msg_ne = tosend.c_str();

            if (send(new_fd, msg_ne, strlen(msg_ne), 0) == -1)
            {
               perror("send");
            }
            printf("%s do not exist. Send a reply to the client.\n", notExistNames.c_str());

            if (requestA.size() == 0 && requestB.size() == 0)
            {
               std::string msg_close = CLOSE_REQUEST;
               std::string msg_close_end = msg_close + MSG_END;
               const char *msg_ce = msg_close_end.c_str(); // 1\n3
               if (send(new_fd, msg_ce, strlen(msg_ce), 0) == -1)
               {
                  perror("send");
               }
               // printf("Sending close request %s to client\n", msg_close);
               continue;
            }
            else
            {
               std::string msg_cont = CONTINUE_REQUEST;
               std::string msg_cont_end = msg_cont + MSG_END;
               const char *msg_ct = msg_cont_end.c_str(); // 2\n3
               if (send(new_fd, msg_ct, strlen(msg_ct), 0) == -1)
               {
                  perror("send");
               }
            }
         }

         int server_to_request = 0;
         // send names to A and B
         if (requestA.size() > 0)
         {
            server_to_request++;
            requestAstr = namesVector_to_str(requestA);
            const char *msg_a = requestAstr.c_str();

            struct sockaddr_in A_addr;
            A_addr.sin_family = AF_INET;
            inet_pton(AF_INET, IP_addr, &A_addr.sin_addr.s_addr);
            A_addr.sin_port = htons(A_UDP_PORT);

            if ((numbytes = sendto(sockfd_u, msg_a, strlen(msg_a), 0, (sockaddr *)&A_addr, sizeof(A_addr))) == -1)
            {
               perror("fail to send request to serverA");
               exit(1);
            }

            printf("Found %s located at ServerA. Send to server A.\n", requestAstr.c_str());

            if (requestABstr.empty())
            {
               requestABstr += requestAstr;
            }
            else
            {
               requestABstr += ", " + requestAstr;
            }
         }

         if (requestB.size() > 0)
         {
            server_to_request++;
            requestBstr = namesVector_to_str(requestB);
            const char *msg_a = requestBstr.c_str();

            struct sockaddr_in B_addr;
            B_addr.sin_family = AF_INET;
            inet_pton(AF_INET, IP_addr, &B_addr.sin_addr.s_addr);
            B_addr.sin_port = htons(B_UDP_PORT);

            if ((numbytes = sendto(sockfd_u, msg_a, strlen(msg_a), 0, (sockaddr *)&B_addr, sizeof(B_addr))) == -1)
            {
               perror("fail to send request to serverB");
               exit(1);
            }

            printf("Found %s located at ServerB. Send to server B.\n", requestBstr.c_str());

            if (requestABstr.empty())
            {
               requestABstr += requestBstr;
            }
            else
            {
               requestABstr += ", " + requestBstr;
            }
         }

         // get the two responses back from A and B
         int recvd_result = 0;
         std::vector<std::vector<int>> resultA;
         std::vector<std::vector<int>> resultB;

         while (1)
         {
            addr_len_u = sizeof their_addr_u;
            if ((numbytes_u = recvfrom(sockfd_u, buf_u, MAXBUFLEN - 1, 0,
                                       (struct sockaddr *)&their_addr_u, &addr_len_u)) == -1)
            {
               perror("recvfrom");
               exit(1);
            }
            buf_u[numbytes_u] = '\0';

            std::vector<int> singleSlot;
            if (buf_u[numbytes_u - 1] == 'A')
            {
               resultA = slotStr_to_vector(buf_u);
               recvd_result++;
               printf("Main Server received from server A the intersection result using UDP over port %s:\n[%s]\n",
                      M_UDP_PORT, slotVector_to_str(resultA).c_str());
            }

            else if (buf_u[numbytes_u - 1] == 'B')
            {
               resultB = slotStr_to_vector(buf_u);
               recvd_result++;
               printf("Main Server received from server B the intersection result using UDP over port %s:\n[%s]\n",
                      M_UDP_PORT, slotVector_to_str(resultB).c_str());
            }

            if (recvd_result == server_to_request)
            {
               break;
            }
         }

         // work out the workable interval and store it to msg
         std::vector<std::vector<int>> final_result;

         if (resultA.size() == 0)
         {
            final_result = resultB;
         }
         else if (resultB.size() == 0)
         {
            final_result = resultA;
         }
         else
         {
            final_result = find_intersection(resultA, resultB);
         }

         requestABstr += '\n';

         const char *msg_names = requestABstr.c_str();

         if (send(new_fd, msg_names, strlen(msg_names), 0) == -1)
         {
            perror("send found names");
         }

         std::string resultStr = slotVector_to_str(final_result);
         std::string msg_result_end = resultStr + '\n' + MSG_END;
         const char *msg_result = msg_result_end.c_str();

         std::cout << "Found the intersection between the results from server A and B: [" << resultStr << ']' << std::endl;

         if (send(new_fd, msg_result, strlen(msg_result), 0) == -1)
         {
            perror("send results");
         }

         std::cout << "Main Server sent the result to the client." << std::endl;
      }

      close(new_fd);
   }

   close(sockfd_u);
   return 0;
}