#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <bits/stdc++.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>


#define PORT "24484"
#define MAXDATASIZE 100 

#define MAX_USERNAME_LEN 30
#define MAX_USERS 10

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) 
   {
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }
   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
//************************* all the pure socket programming code and lines are from Pages 34 - 42 of the Beej’s Guide to Network Programming ****************

  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  int client_TCP_PORT;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  struct sockaddr_in my_addr;
  socklen_t len = sizeof(my_addr);

  if ((rv = getaddrinfo("127.0.0.1", PORT, &hints, &servinfo)) != 0) 
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) 
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
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

    /*Retrieve the locally-bound name of the specified socket and store it in the
    sockaddr structure*/
    if(getsockname(sockfd, (struct sockaddr*)&my_addr, &len) == -1)
    {
   
        perror("getsockname");
        exit(1);
    }
    else
    {
      client_TCP_PORT = ntohs(my_addr.sin_port);
      
    }

    break;
  }

  if (p == NULL) 
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);

  printf("Client is up and running\n");
  cout<<endl;

  freeaddrinfo(servinfo);

  printf("Please enter the usernames to check schedule availability:\n");
  cout<<endl;
   
  while(true) 
  {
    char input[1000];
    char usernames[10][100];
    int i = 0, j = 0, k = 0;
    cout<<endl; 

    
    fgets(input, 1000, stdin);

    int len = strlen(input);
    int p, q;

    // find the first non-space character
    for (p = 0; p < len; p++) 
    {
        if (!isspace(input[p])) 
        {
            break;
        }
    }

    // move all remaining characters to the beginning of the string
    for (q = 0; p < len; p++, q++) 
    {
        input[q] = input[p];
    }
    input[q] = '\0'; // terminate the string

    while (input[i] != '\0') 
    {
      if (input[i] == ' ' || input[i] == '\n') 
      {
        usernames[j][k] = '\0';
        j++;
        k = 0;
      } 
      else 
      {
        usernames[j][k] = input[i];
        k++;
      }
      i++;
    }

    usernames[j][k] = '\0';

    if(input[0] == ' ' || input[0] == '\n' || input[0] == '\0')
    {
      j = 0;
    }

    sleep(1);
    send(sockfd, &input, strlen(input), 0);
    //sleep(1);

  
    int numberOfUsernamesEntered = j;

    cout<<endl;

    printf("Client finished sending the usernames to Main Server.\n");

    int numberOfNonUsernames;
    char invalidUsernames[MAX_USERS][MAX_USERNAME_LEN];
    for (int i = 0; i < MAX_USERS; i++) 
    {
      for (int j = 0; j < MAX_USERNAME_LEN; j++) 
      {
        invalidUsernames[i][j] = '\0';
      }
    }

    numbytes = recv(sockfd, &numberOfNonUsernames, sizeof(int), 0);

    if(numberOfNonUsernames>0)
    {
      int numberOfInvalidUsernames = numberOfNonUsernames;
      int count_tcp = 0;
      char invalidBuf[MAXDATASIZE];

      numbytes = recv(sockfd, invalidBuf, MAXDATASIZE-1, 0);
      invalidBuf[numbytes] = '\0';


      
      int userCountInvalid = 0;

      char* single_username = strtok(invalidBuf, " ");
      while (single_username != NULL && userCountInvalid < numberOfNonUsernames) 
      {
        strncpy(invalidUsernames[userCountInvalid], single_username, MAX_USERNAME_LEN - 1);
        invalidUsernames[userCountInvalid][MAX_USERNAME_LEN - 1] = '\0';
        userCountInvalid++;
        single_username = strtok(NULL, " ");
        if (single_username != NULL && *single_username == '\0') 
        {
          break;
        }
      }

      cout<<endl;
      printf("Client received the reply from Main Server using TCP over port %d: ", client_TCP_PORT);
      for (int i = 0; i < numberOfNonUsernames-1; i++) 
      {
        printf("%s, ",invalidUsernames[i]);
      }
      printf("%s do not exist.",invalidUsernames[numberOfNonUsernames-1]);
      cout<<endl;
      cout<<endl;
    }


    int numberOfValidUsernames = numberOfUsernamesEntered - numberOfNonUsernames;
    char validUsernames[MAX_USERS][MAX_USERNAME_LEN];
    int flag = 1;
    int count_valid = 0;

    if(numberOfValidUsernames > 0)
    {
      for (int i = 0; i < numberOfUsernamesEntered; i++) 
      {
        for (int j = 0; j < numberOfNonUsernames; j++)
        {
          if(strcmp(usernames[i], invalidUsernames[j]) == 0)
          {
            flag = 0;
          }
        }
        if(flag == 1)
        {
          strcpy(validUsernames[count_valid], usernames[i]);
          count_valid++;
        }
        flag = 1;  
      }
    }


    char buf_interval[MAXDATASIZE];
    numbytes = recv(sockfd, &buf_interval, MAXDATASIZE-1, 0);
    
    buf_interval[numbytes] = '\0';
    
    vector<vector<int> > intervals;

    string availability_str(buf_interval);
      
    stringstream availability_ss(availability_str);

    int start_time, end_time;

    while (availability_ss >> start_time >> end_time) 
    {
      intervals.push_back({start_time, end_time});
    }
    //cout << "numberOfValidUsernames = " << numberOfValidUsernames << endl;

    if(intervals.size()>0)
    {
      cout << "Client received the reply from Main Server using TCP over port " << client_TCP_PORT << ": Time intervals [";
      for (size_t i = 0; i < intervals.size()-1; ++i) 
      {
        if(intervals[i][0] != 0)
        {
          cout << "[" << intervals[i][0] << ", " << intervals[i][1] << "],";
        }
      }
      if(intervals[intervals.size()-1][0] != 0)
      {
        cout << "[" << intervals[intervals.size()-1][0] << ", " << intervals[intervals.size()-1][1] << "]";
      }

      cout << "]";
    
      cout << " works for ";
  
      for (int i = 0; i < numberOfValidUsernames-1; i++) 
      {
        cout << validUsernames[i] << ", ";
      }
        
      printf("%s.\n", validUsernames[numberOfValidUsernames-1]);

      cout << endl;
    }

    else if(intervals.size()<=0 && numberOfValidUsernames>0)
    {
      cout << "Client received the reply from Main Server using TCP over port " << client_TCP_PORT << ": Time intervals [] works for ";
      for (int i = 0; i < numberOfValidUsernames-1; i++) 
      {
        cout << validUsernames[i] << ", ";
      }

      printf("%s.\n", validUsernames[numberOfValidUsernames-1]);

      cout << endl;
    }

  printf("-----Start a new request-----\n");
  printf("Please enter the usernames to check schedule availability:\n");
  cout<<endl;      
    
  }
  return 0; 
}