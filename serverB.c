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
#include <iterator>
#include <algorithm>
#include <utility>
#include <cstdlib>

using namespace std;

#define PORT 22484

#define MAX_FILE_SIZE 30000

#define MAX_USERNAME_LEN 50
#define MAX_USERS 10
#define MAXLINE 1024


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


// function to trim a string of leading and trailing spaces
string trim(string stringToBeTrimmed)
{
  int start_of_non_space = 0;
  int end_of_non_space = stringToBeTrimmed.size()-1;
  int i = 0;

  while(i < stringToBeTrimmed.size())
  {
    if(stringToBeTrimmed[i] != ' ')
    {
      start_of_non_space = i;
      break;
    }
    i++;
  }

  int j = stringToBeTrimmed.size()-1;
  while(j >= 0)
  {
    if(stringToBeTrimmed[j] != ' ')
    {
      end_of_non_space = j;
      break;
    }
    j--;
  }

  string trimmedString = "";
  int k = start_of_non_space;

  while(k <= end_of_non_space)
  {
    trimmedString = trimmedString + stringToBeTrimmed[k];
    k++;
  }
  
  return trimmedString;
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

//********************************************* reading the file and storing the usernames in a char usernames [] array

  const char* filename_to_be_read = "b.txt";

  char usernames[MAX_FILE_SIZE];
  int usernames_index = 0;
  int usernameCountB = 0;

  ifstream file(filename_to_be_read);
  if (file.is_open()) 
  {
    char user_aval_sentence[MAX_FILE_SIZE];

    while (file.getline(user_aval_sentence, MAX_FILE_SIZE)) 
    {
      // strchr searches for the first occurence of ';'
      char* semi_colon_loc = strchr(user_aval_sentence, ';');
      if (semi_colon_loc != nullptr) 
      {
        // Remove trailing spaces from the username
        int username_length = semi_colon_loc - user_aval_sentence;
        while (username_length > 0 && user_aval_sentence[username_length - 1] == ' ')
        {
          username_length--;
        }
        strncpy(&usernames[usernames_index], user_aval_sentence, username_length);
        usernames_index += username_length;
        usernames[usernames_index++] = ' ';
        usernameCountB++;
      }
    }
    
    file.close();
  }

  // Remove trailing space from the usernames buffer
  if (usernames_index > 0 && usernames[usernames_index - 1] == ' ')
  {
    usernames_index--;
  }
  usernames[usernames_index] = '\0';


//************************* all the pure socket programming code and lines are from Pages 34 - 42 of the Beej’s Guide to Network Programming ****************

  int sockfd, numbytes, numbytes2, rv; 
  struct sigaction sa;
  char s[INET6_ADDRSTRLEN];
  struct sockaddr_in servaddr, cliaddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) 
  {
    perror("listener: socket");
    exit(EXIT_FAILURE);
  }

  cout << endl;

//***************************** server is up and running now *************************************************************
  printf("Server B is up and running using UDP on port %d\n", PORT);
  cout << endl;

  cliaddr.sin_family = AF_INET;
  cliaddr.sin_port = htons(23484);
  cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  memset(&servaddr, 0, sizeof(servaddr));
  
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  servaddr.sin_port = htons(PORT);
       
    
  if(bind(sockfd,(const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1 )
  {
    close(sockfd);
    perror("listener: bind");
    exit(EXIT_FAILURE);
  }

  char buffer[1000];

  socklen_t len;
  int n;
   
  len = sizeof(cliaddr);  

//***************************** sending the number of usernames in b.txt to the Main Server
  sendto(sockfd, &usernameCountB, sizeof(usernameCountB), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

  sleep(0.5);

//***************************** sending all the usernames in b.txt to the Main Server
  sendto(sockfd, usernames, sizeof(usernames), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

  std::cout<<"Server B finished sending a list of usernames to Main Server."<<std::endl;
  cout << endl; 

  while(true)
  {
    char validBuffer[5000];

    int count = 0;
    char existingUsernames[MAX_USERS][MAX_USERNAME_LEN]; // array of valid usernames, that is, usernames entered by the client user that are also in b.txt

    int numberOfExistingUsernames = 0;

//********************************* receiving the number of valid usernames, that is, usernames entered by the client user that are also in b.txt
    recvfrom(sockfd, (int *)&numberOfExistingUsernames, sizeof(int), MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);

//*********************************** if none of the usernames entered by the client user are in b.txt, then the Main server will not send the usernames to Server B
    if(numberOfExistingUsernames > 0)
    {

      // receiving the valid usernames from Main server, only if one or more of the usernames entered by the client user are in b.txt
      numbytes = recvfrom(sockfd, (char *)validBuffer, sizeof(validBuffer), MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
      validBuffer[numbytes] = '\0';

      std::cout << "Server B received the usernames from Main Server using UDP over port " << PORT << endl;
      cout << endl;

      int keepUserCount = 0;

      // extracting usernames from the char[] array and storing them in the char[][] array
      char* single_username = strtok(validBuffer, " ");
      while (single_username != NULL && keepUserCount < numberOfExistingUsernames) 
      {
        strncpy(existingUsernames[keepUserCount], single_username, MAX_USERNAME_LEN - 1);
        existingUsernames[keepUserCount][MAX_USERNAME_LEN - 1] = '\0';
        keepUserCount++;
        single_username = strtok(NULL, " ");
        if (single_username != NULL && *single_username == '\0') 
        {
          break;
        }
      }

      // map to store all usernames and availabilities
      map<string, vector<vector<int> > > user_interval_map;


      ifstream file_to_be_read("b.txt");
      string username_aval_line;
    
      while (getline(file_to_be_read, username_aval_line)) 
      {
        stringstream total_ss(username_aval_line);
        string username_untrim;
        getline(total_ss, username_untrim, ';');
        string username;

        // trimming the username of extra spaces using trim() function       
        username = trim(username_untrim);
        
        // extracting the availabilities and replacing '[', ']', ';'' by space.
        string availability_str;
        getline(total_ss, availability_str);
        availability_str = availability_str.substr(2, availability_str.size() - 4);
        replace(availability_str.begin(), availability_str.end(), ',', ' ');
        replace(availability_str.begin(), availability_str.end(), ']', ' ');
        replace(availability_str.begin(), availability_str.end(), '[', ' ');
        stringstream availability_ss(availability_str);

        // structure to store the intervals of one user
        vector<vector<int> > intervals;

        int t_start, t_end;

        while (availability_ss >> t_start >> t_end) 
        {
          intervals.push_back({t_start, t_end});
        }
  
        // inserting one username and their corresponding availability as one entry into the map
        user_interval_map.insert(pair<string, vector<vector<int> > >(username, intervals));
      }      

      // map to store only valid usernames and availabilities (that is, those usernames entered by the client user that are also in b.txt)
      map<string, vector<vector<int> > > valid_user_interval_map;

      map<string, vector<vector<int> > >::iterator all_it;
      
      for (all_it = user_interval_map.begin(); all_it != user_interval_map.end(); all_it++) 
      {
        for (int i = 0; i < numberOfExistingUsernames; i++) 
        {
          if(all_it->first == existingUsernames[i])
          {
            valid_user_interval_map.insert(pair<string, vector<vector<int> > >(all_it->first, all_it->second));
          }
        }     
      }

      // iterating through the availabilities to find the intersection
      map<string, vector<vector<int> > >::iterator valid_it;

      valid_it = valid_user_interval_map.begin();

      vector<vector<int> > intervals1 = valid_it->second;

      valid_it++;

      while(valid_it != valid_user_interval_map.end()) 
      {
        vector<vector<int> > intervals2 = valid_it->second;

        intervals1 = findIntersectionOfAvailabilities(intervals1, intervals2, intervals1.size(), intervals2.size());

        valid_it++;
      }

    
      // printing the intersection result
      cout << "Found the intersection result: [";

      if(intervals1.size()>0)
      {
        for (size_t i = 0; i < intervals1.size()-1; ++i) 
        {
          if(intervals1[i][0] != 0)
          {
            cout << "[" << intervals1[i][0] << ", " << intervals1[i][1] << "],";
          }
        }
        if(intervals1[intervals1.size()-1][0] != 0)
        {
          cout << "[" << intervals1[intervals1.size()-1][0] << ", " << intervals1[intervals1.size()-1][1] << "]";
        }
      }
      cout << "] for ";
    
      for (int i = 0; i < numberOfExistingUsernames-1; i++) 
      {
        cout << existingUsernames[i] << ", ";
      }
        
      printf("%s.\n", existingUsernames[numberOfExistingUsernames-1]);

      cout << endl;


      // converting vector<vector<int> > intervals1 to char[]
      char intervals_str[2000] = {0};

      for (vector<int> row : intervals1) 
      {
        for (int element : row) 
        {
          char ele_temp[100];
          sprintf(ele_temp, "%d ", element);
          strcat(intervals_str, ele_temp);
        }       
      }

//************************************* sending the final intersection of availabilities of valid usernames to the Main server
      sendto(sockfd, intervals_str, strlen(intervals_str), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));

      cout << "Server B finished sending the response to Main Server." << endl;
    }

  }
  
  return 0;
}