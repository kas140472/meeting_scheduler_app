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

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <utility>
#include <cstdlib>

using namespace std;

#define TCP_PORT "24484"
#define UDP_PORT "23484"
#define MAXDATASIZE 100
#define BACKLOG 10   

#define SERVER_A_PORT "21484"
#define SERVER_B_PORT "22484"

#define MAX_USERNAME_LEN 100
#define MAX_USERS 10


// function to compute intersection of two interval lists
vector<vector<int>> findIntersectionOfAvailabilities(vector<vector<int>>& i1, vector<vector<int>>& i2, int i1_size, int i2_size) 
{
  vector<vector<int>>intersection_result;
  int i1_pointer=0;
  int i2_pointer=0;
  int first_pair_index = 0;
  int second_pair_index = 1;
  
  if( i1_size==0 || i2_size==0 )
  {
    return intersection_result;
  }
  
  while(true)
  {
    vector<int> temp;

    if(i1_pointer==i1_size || i2_pointer==i2_size)
    {
      return intersection_result;
    }
    // no overlap
    if(i2[i2_pointer][first_pair_index] >= i1[i1_pointer][second_pair_index] || i1[i1_pointer][first_pair_index] >= i2[i2_pointer][second_pair_index])
    {
      if( i1[i1_pointer][second_pair_index]>i2[i2_pointer][second_pair_index] )
      {
        i2_pointer++;
      }
      else
      {
        i1_pointer++;
      }
    }
    // overlap
    else
    {
      if(max(i1[i1_pointer][first_pair_index], i2[i2_pointer][first_pair_index]) != min(i1[i1_pointer][second_pair_index], i2[i2_pointer][second_pair_index]))
      {
        temp.push_back(max(i1[i1_pointer][first_pair_index], i2[i2_pointer][first_pair_index]));
        temp.push_back(min(i1[i1_pointer][second_pair_index], i2[i2_pointer][second_pair_index]));
      }
      else
      {
        temp.push_back(0);
        temp.push_back(0);
      }

      intersection_result.push_back(temp);
      temp.clear();
                
      if(i1[i1_pointer][second_pair_index] > i2[i2_pointer][second_pair_index])
      {
        i2_pointer++;
      }
      else
      {
        i1_pointer++;
      }
    }
  }
  return intersection_result;
}


//******************************** the functions sigchld_handler and *get_in_addr are directly from Beej's guide ********************************
void sigchld_handler(int s)
{
  int saved_errno = errno;

  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) 
  {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//***************************************************************************************************************************************************

int main(void)
{
//************************* all the pure socket programming code and lines are from Pages 34 - 42 of the Beej’s Guide to Network Programming ****************

  int udp_numbytes;
  struct addrinfo hints, *servinfo, *p;

  int udp_sockfd;
    
  struct addrinfo udp_hints, *udp_servinfo, *udp_p;
  int udp_rv;
  struct sockaddr_storage udp_their_addr;
      
  struct sigaction sa;
  int yes=1;  
  
  int port_number;
  
  socklen_t udp_addr_len;
  char udp_s[INET6_ADDRSTRLEN];

  memset(&udp_hints, 0, sizeof udp_hints);
  udp_hints.ai_family = AF_UNSPEC;
  udp_hints.ai_socktype = SOCK_DGRAM;
  udp_hints.ai_flags = AI_PASSIVE;

  if((udp_rv = getaddrinfo(NULL, UDP_PORT, &udp_hints, &udp_servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_rv));
    return 1;
  }

  for(udp_p = udp_servinfo; udp_p != NULL; udp_p = udp_p->ai_next) 
  {
    if ((udp_sockfd = socket(udp_p->ai_family, udp_p->ai_socktype, udp_p->ai_protocol)) == -1) 
    {
      perror("listener: socket");
      continue;
    }
    if (bind(udp_sockfd, udp_p->ai_addr, udp_p->ai_addrlen) == -1) 
    {
      close(udp_sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }

  if (udp_p == NULL) 
  {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(udp_servinfo);

  // server A
  struct addrinfo udp_A_hints, *udp_A_servinfo, *udp_A_p;
  int udp_A_rv;

  memset(&udp_A_hints, 0, sizeof udp_A_hints);
  udp_A_hints.ai_family = AF_UNSPEC;
  udp_A_hints.ai_socktype = SOCK_DGRAM;

  if((udp_A_rv = getaddrinfo("127.0.0.1", SERVER_A_PORT, &udp_A_hints, &udp_A_servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_A_rv));
    return 1;
  }

  udp_A_p = udp_A_servinfo;

  // server B
  struct addrinfo udp_B_hints, *udp_B_servinfo, *udp_B_p;
  int udp_B_rv;

  memset(&udp_B_hints, 0, sizeof udp_B_hints);
  udp_B_hints.ai_family = AF_UNSPEC;
  udp_B_hints.ai_socktype = SOCK_DGRAM;

  if((udp_B_rv = getaddrinfo("127.0.0.1", SERVER_B_PORT, &udp_B_hints, &udp_B_servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(udp_B_rv));
    return 1;
  }

  udp_B_p = udp_B_servinfo;

//*****************************************************************************************************************8*************************

  printf("Main Server is up and running.\n");

//******************************************************** getting all usernames from Server A

  // store all usernames in a.txt in a char[] array
  char udp_bufA[5000]; 

  // store all usernames in a.txt in a char[][] array
  char usernamesA[MAX_USERS][MAX_USERNAME_LEN];

  int countA = 0;
  int numberOfUsernamesInA = 0;

  // receiving the number of usernames present in a.txt
  udp_numbytes = recvfrom(udp_sockfd, &numberOfUsernamesInA, sizeof(int), 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);

  int numberOfUsernamesInServerA = numberOfUsernamesInA;

  // receiving all the usernames present in a.txt in char[] form
  udp_numbytes = recvfrom(udp_sockfd, udp_bufA, 5000, 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);
  udp_bufA[udp_numbytes] = '\0';

  int i = 0, j = 0, k = 0; 

  // converting usernames from char[] form to char[][] form
  while (udp_bufA[i] != '\0') 
  {
    if (udp_bufA[i] == ' ' || udp_bufA[i] == '\n') 
    {
      usernamesA[j][k] = '\0';
      j++;
      k = 0;
    } 
    else 
    {
      usernamesA[j][k] = udp_bufA[i];
      k++;
    }
    i++;
  }

  usernamesA[j][k] = '\0';

  cout <<"Main Server received the username list from server A using UDP over port " << UDP_PORT << "." << endl;
  cout << endl;

//******************************************************** getting all usernames from Server B

  // store all usernames in b.txt in a char[] array
  char udp_bufB[5000];

   // store all usernames in a.txt in a char[][] array
  char usernamesB[MAX_USERS][MAX_USERNAME_LEN];

  int countB = 0;
  int numberOfUsernamesInB = 0;

  // receiving the number of usernames present in b.txt
  udp_numbytes = recvfrom(udp_sockfd, &numberOfUsernamesInB, sizeof(int), 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);
  int numberOfUsernamesInServerB = numberOfUsernamesInB;

  // receiving all the usernames present in b.txt in char[] form
  udp_numbytes = recvfrom(udp_sockfd, udp_bufB, 300, 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);
  udp_bufB[udp_numbytes] = '\0';

  i = 0, j = 0, k = 0; 

  // converting usernames from char[] form to char[][] form
  while (udp_bufB[i] != '\0') 
  {
    if (udp_bufB[i] == ' ' || udp_bufB[i] == '\n') 
    {
      usernamesB[j][k] = '\0';
      j++;
      k = 0;
    } 
    else 
    {
      usernamesB[j][k] = udp_bufB[i];
      k++;
    }
    i++;
  }

  usernamesB[j][k] = '\0';

  cout <<"Main Server received the username list from server B using UDP over port " << UDP_PORT << "." << endl;
  cout << endl;



// **************************************************************************************** TCP: Client-Server M
//************************* all the pure socket programming code and lines are from Pages 34 - 42 of the Beej’s Guide to Network Programming ****************

  int sockfd, new_fd, numbytes;       

  struct sockaddr_storage their_addr;  
  socklen_t sin_size;     

 
  char s[INET6_ADDRSTRLEN];
  int rv; 


  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;    

  if ((rv = getaddrinfo(NULL, TCP_PORT, &hints, &servinfo)) != 0) 
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) 
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

  freeaddrinfo(servinfo);

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

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) 
  {
    perror("sigaction");
    exit(1);
  }

  cout << endl;

  sin_size = sizeof their_addr;
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
//***************************************************************************************************************************************************

  while(true)
  {
    // array to store usernames entered by the user
    char userEnteredUsernames[MAX_USERS][MAX_USERNAME_LEN];

    int count_tcp = 0;
    
    char tcp_buf[MAXDATASIZE]; 
    int i = 0, j = 0, k = 0; 

    
    int numberOfUsernamesEnteredByUser;

    // receiving all the usernames entered by the user in a char[] array
    numbytes = recv(new_fd, &tcp_buf, MAXDATASIZE-1, 0);
    tcp_buf[numbytes] = '\0';

    // converting usernames from char[] form to char[][] form 
    while (tcp_buf[i] != '\0') 
    {
      if (tcp_buf[i] == ' ' || tcp_buf[i] == '\n') 
      {
        userEnteredUsernames[j][k] = '\0';
        j++;
        k = 0;
      } 
      else 
      {
        userEnteredUsernames[j][k] = tcp_buf[i];
        k++;
      }
      i++;
    }

    userEnteredUsernames[j][k] = '\0';
    numberOfUsernamesEnteredByUser = j;


    cout << "Main Server received the request from the client using TCP over port " << TCP_PORT << "." << endl;
    cout << endl;




  //********************************************************************************************************************************************
  // Finding out which usernames exist and which don't, printing accordingly, and sending the valid usernames to servers A and B via UDP

    char nonUsernamesA[MAX_USERS][MAX_USERNAME_LEN];
    char existingUsernamesA[MAX_USERS][MAX_USERNAME_LEN];
    int count_useA = 0;
    int count_existA = 0;
    int flagA = 1;

    for (int i = 0; i < numberOfUsernamesEnteredByUser; i++) 
    {
      for (int j = 0; j < numberOfUsernamesInServerA; j++)
      {
        if(strcmp(userEnteredUsernames[i], usernamesA[j]) == 0)
        {
          flagA = 0;
          strcpy(existingUsernamesA[count_existA], userEnteredUsernames[i]);
          count_existA++;
        }
      }
      if(flagA == 1)
      {
        strcpy(nonUsernamesA[count_useA], userEnteredUsernames[i]);
        count_useA++;
      }
      flagA = 1;
    }

//***********************************converting char[][] array existingUsernamesA to char[] existingUsernamesA_str

    char existingUsernamesA_str[5000];
    int usernames_indexA = 0;
    for (int i = 0; i < count_existA; i++) 
    {
      strncpy(&existingUsernamesA_str[usernames_indexA], existingUsernamesA[i], MAX_USERNAME_LEN);
      usernames_indexA += strlen(existingUsernamesA[i]);
      existingUsernamesA_str[usernames_indexA++] = ' ';
    }

  // Null-terminate the resulting string of usernames
    existingUsernamesA_str[usernames_indexA] = '\0';
//****************************************************************************************************************888


    char nonUsernamesB[MAX_USERS][MAX_USERNAME_LEN];
    char existingUsernamesB[MAX_USERS][MAX_USERNAME_LEN];
    int count_useB = 0;
    int count_existB = 0;
    int flagB = 1;

    for (int i = 0; i < numberOfUsernamesEnteredByUser; i++) 
    {
      for (int j = 0; j < numberOfUsernamesInServerB; j++)
      {
        if(strcmp(userEnteredUsernames[i], usernamesB[j]) == 0)
        {
          flagB = 0;
          strcpy(existingUsernamesB[count_existB], userEnteredUsernames[i]);
          count_existB++;
        }
      }
      if(flagB == 1)
      {
        strcpy(nonUsernamesB[count_useB], userEnteredUsernames[i]);
        count_useB++;
      }
      flagB = 1;
      
    }


//***********************************converting char[][] array existingUsernamesB to char[] existingUsernamesB_str
    char existingUsernamesB_str[5000];
    int usernames_indexB = 0;
    
    for (int i = 0; i < count_existB; i++) 
    { 
      strncpy(&existingUsernamesB_str[usernames_indexB], existingUsernamesB[i], MAX_USERNAME_LEN);
      usernames_indexB += strlen(existingUsernamesB[i]);
      existingUsernamesB_str[usernames_indexB++] = ' ';
    }

  // Null-terminate the resulting string of usernames
    existingUsernamesB_str[usernames_indexB] = '\0';
//*********************************************************************************************************************


//************************ usernames which don't exist in a.txt or b.txt
    char nonUsernamesAB[MAX_USERS][MAX_USERNAME_LEN];
    int count_useAB = 0;

    
    for (int i = 0; i < count_useA; i++) 
    {
      for (int j = 0; j < count_useB; j++)
      {
        if(strcmp(nonUsernamesA[i], nonUsernamesB[j]) == 0)
        {
          strcpy(nonUsernamesAB[count_useAB], nonUsernamesA[i]);
          count_useAB++;
        }
      }
    }

//***********************************converting char[][] array nonUsernamesAB to char[] nonUsernamesAB_str
    char nonUsernamesAB_str[5000];
    int nonUsernames_indexAB = 0;
    for (int i = 0; i < count_useAB; i++) 
    {
      strncpy(&nonUsernamesAB_str[nonUsernames_indexAB], nonUsernamesAB[i], MAX_USERNAME_LEN);
      nonUsernames_indexAB += strlen(nonUsernamesAB[i]);
      nonUsernamesAB_str[nonUsernames_indexAB++] = ' ';
    }

  // Null-terminate the resulting string of usernames
    nonUsernamesAB_str[nonUsernames_indexAB] = '\0';

//****************************************************************************************************************
//******************************************************************************************************************

    for (int i = 0; i < count_useAB-1; i++) 
    {
      std::cout<< nonUsernamesAB[i] << ", ";
    }

    if(count_useAB > 0)
    {
      std::cout << nonUsernamesAB[count_useAB-1] << " do not exist. Send a reply to the client." << std::endl;
      cout << endl;
    }
    else
    {
      cout << endl;
    }
    
    
    // sending the number of invalid usernames to client
    send(new_fd, &count_useAB, sizeof(count_useAB), 0);

    // sending all the invalid usernames to client if and only if there are any invalid usernames entered
    if(count_useAB > 0)
    {
      sleep(1);
      numbytes = send(new_fd, nonUsernamesAB_str, strlen(nonUsernamesAB_str), 0);
    }

    cout<<endl;

    // printing valid usernames found in a.txt
    if(count_existA > 0)
    {
      cout<< "Found ";

      for (int i = 0; i < count_existA-1; i++) 
      {
        std::cout<< existingUsernamesA[i] << ", ";
      }
      
      std::cout << existingUsernamesA[count_existA-1] << " located at Server A. Send to Server A." << std::endl;
    }

    

    cout << endl;

    // printing valid usernames found in b.txt
    if(count_existB > 0)
    {
      cout<< "Found ";

      for (int i = 0; i < count_existB-1; i++) 
      {
        std::cout<< existingUsernamesB[i] << ", ";
      }
      
      std::cout << existingUsernamesB[count_existB-1] << " located at Server B. Send to Server B." << std::endl;
    }

    cout << endl;

//****************************************************sending existingUsernamesA_str
    struct sockaddr_in servaddr,servaddr1;
 
    sendto(udp_sockfd, &count_existA, sizeof(count_existA), 0, udp_A_p->ai_addr, udp_A_p->ai_addrlen);

    if(count_existA > 0)
    {
      sleep(1);

      sendto(udp_sockfd, existingUsernamesA_str, strlen(existingUsernamesA_str), 0, udp_A_p->ai_addr, udp_A_p->ai_addrlen);
    }
//*******************************************************************************************************************
//****************************************************sending existingUsernamesB_str
    
 
    sendto(udp_sockfd, &count_existB, sizeof(count_existB), 0, udp_B_p->ai_addr, udp_B_p->ai_addrlen);

    if(count_existB > 0)
    {
      sleep(1);

      sendto(udp_sockfd, existingUsernamesB_str, strlen(existingUsernamesB_str), 0, udp_B_p->ai_addr, udp_B_p->ai_addrlen);
    }

    //*********************************************************************************************************************************
    // receiving intersection from servers A and B via UDP

    int udp_numbytesA, udp_numbytesB;


    char interval_bufA[100];
    if(count_existA > 0)
    {
      udp_numbytesA = recvfrom(udp_sockfd, &interval_bufA, 100, 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);
      interval_bufA[udp_numbytesA] = '\0';
    }

    char interval_bufB[100];    
    if(count_existB > 0)
    {
      udp_numbytesB = recvfrom(udp_sockfd, &interval_bufB, 100, 0, (struct sockaddr *)&udp_their_addr, &udp_addr_len);
      interval_bufB[udp_numbytesB] = '\0';
    }


    //*******************************************************************************************************************************************
    // finding intersection between intersections sent by A and B

    vector<vector<int> > intervalsA;

    // converting char[] interval_bufA to vector<vector<int> > intervalsA and printing the intervals

    if(count_existA > 0)
    {
      string availability_strA(interval_bufA);
      
      stringstream availability_ssA(availability_strA);

      

      int start_timeA, end_timeA;

      while (availability_ssA >> start_timeA >> end_timeA) 
      {
        intervalsA.push_back({start_timeA, end_timeA});
      }

      cout << "Main Server received from server A the intersection result using UDP over port "<< UDP_PORT << ": [";

      if(intervalsA.size()>0)
      {
        for (size_t i = 0; i < intervalsA.size()-1; ++i) 
        {
          if(intervalsA[i][0] != 0)
          {
            cout << "[" << intervalsA[i][0] << ", " << intervalsA[i][1] << "],";
          }
        }
        if(intervalsA[intervalsA.size()-1][0] != 0)
        {
          cout << "[" << intervalsA[intervalsA.size()-1][0] << ", " << intervalsA[intervalsA.size()-1][1] << "]";
        }
      }
      cout << "]" << endl;
      cout<<endl;
    }
    else
    {
      intervalsA.push_back({0,0});
    }

    vector<vector<int> > intervalsB;
    
    // converting char[] interval_bufB to vector<vector<int> > intervalsB and printing the intervals

    if(count_existB > 0)
    {
      string availability_strB(interval_bufB);
      
      stringstream availability_ssB(availability_strB);
      

      int start_timeB, end_timeB;

      while (availability_ssB >> start_timeB >> end_timeB) 
      {
        intervalsB.push_back({start_timeB, end_timeB});
      }


      cout << "Main Server received from server B the intersection result using UDP over port "<< UDP_PORT << ": [";

      if(intervalsB.size()>0)
      {
        for (size_t i = 0; i < intervalsB.size()-1; ++i) 
        {
          if(intervalsB[i][0] != 0)
          {
            cout << "[" << intervalsB[i][0] << ", " << intervalsB[i][1] << "],";
          }
        }
        if(intervalsB[intervalsB.size()-1][0] != 0)
        {
          cout << "[" << intervalsB[intervalsB.size()-1][0] << ", " << intervalsB[intervalsB.size()-1][1] << "]";
        }
      }
      cout << "]" << endl;
      cout<<endl;
    }
    else
    {
      intervalsB.push_back({0,0});
    }

    int exist = 0;

// finding the intersection of A and B intervals
    vector<vector<int> > intervalsAB;

    if(count_existA > 0 && count_existB > 0)
    {
      intervalsAB = findIntersectionOfAvailabilities(intervalsA, intervalsB, intervalsA.size(), intervalsB.size());
      exist = 1;
    }
    else if(count_existA == 0 && count_existB > 0)
    {
      intervalsAB = intervalsB;
      exist = 1;
    }
    else if(count_existB == 0 && count_existA > 0)
    {
      intervalsAB = intervalsA;
      exist = 1;
    }


// sending the final intersection to the client 
    if((count_existB == 0 && count_existA == 0) || intervalsAB.size() == 0)
    {
      intervalsAB.push_back({0,0});
      char intervals_str[2000] = {0};

      for (size_t i = 0; i < intervalsAB.size(); i++) 
      {
        for (size_t j = 0; j < intervalsAB[i].size(); j++) 
        {
          char tempAB[1000];
          sprintf(tempAB, "%d ", intervalsAB[i][j]);
          strcat(intervals_str, tempAB);
        }
      }

      sleep(1);
      send(new_fd, &intervals_str, strlen(intervals_str), 0);
      if(exist == 1)
      {
        cout << "Found the intersection between the results from server A and B: []" << endl;
      }
      cout << "Main Server sent the result to the client." << endl;
    }
    else
    {  
      cout << "Found the intersection between the results from server A and B: [";

      if(intervalsAB.size()>0)
      {
        for (size_t i = 0; i < intervalsAB.size()-1; ++i) 
        {
          if(intervalsAB[i][0] != 0)
          {
            cout << "[" << intervalsAB[i][0] << ", " << intervalsAB[i][1] << "],";
          }
        }
        if(intervalsAB[intervalsAB.size()-1][0] != 0)
        {
          cout << "[" << intervalsAB[intervalsAB.size()-1][0] << ", " << intervalsAB[intervalsAB.size()-1][1] << "]";
        }
      }
      cout << "]" << endl;
      cout<<endl;


      char intervals_str[2000] = {0};

      for (size_t i = 0; i < intervalsAB.size(); i++) 
      {
        for (size_t j = 0; j < intervalsAB[i].size(); j++) 
        {
          char temp[100];
          sprintf(temp, "%d ", intervalsAB[i][j]);
          strcat(intervals_str, temp);
        }
      }
      //cout << "intervals_str = " << intervals_str << endl;
      sleep(1);
      send(new_fd, &intervals_str, strlen(intervals_str), 0);
      cout << "Main Server sent the result to the client." << endl;
    }

  }

  return 0;
}