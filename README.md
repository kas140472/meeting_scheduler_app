## Meeting Scheduler Application using Socket Programming

![img1](https://github.com/kas140472/meeting_scheduler_app/edit/main/serverAB_mixed2.png?raw=true)

An efficient meeting scheduling system using C/C++ on Unix, leveraging 
backend (UDP connection) and main (TCP connection) servers to identify 
optimal meeting times for participants based on their availability. 

6 files: README, Makefile, serverM.c, serverA.c, serverB.c, and client.c
   
Code files: serverM.c, serverA.c, serverB.c, and client.c.

Input Files:
a.txt for backend server A and b.txt for backend server B. 
The two files share the same format as follows:
   The file contains a list of time availability for a group of individuals identified by their
   usernames. The format of the time availability information stored in the file for a specific
   user is provided:
   user3;[[5,10],[11,16]]


How it works:
serverM stores the usernames obtained from servers A and B. Then it
waits for the client to send user-entered usernames. It stores these
usernames and compares them with usernames of server A and B. It figures
out which of the entered usernames exist in Server A and sends them to
Server A. It does the same with Server B. If none of the usernames entered
exist in either A or B or both, then the Main Server does not send the
usernames to that respective server. For the usernames that do not exist 
in either A or B, Server M sends those usernames to the client, which
then prints those usernames as 'not existing'. Server A and B compute the 
intersection of the intervals of the usernames received from Server M. 
They send the result to Server M. Server M coputes the intersection 
between the intersections received from A and B and sends it to client.
   
   
Format of messages:
   Messages between client and Main Server: 
   1. Client sends the list of user-entered usernames to the Main Server. 
      The format is a char[] array of all the usernames. The usernames are 
      formatted in the char[] array exactly as they are entered by the user, 
      with space between any two usernames and no other special characters.
   2. The Main Server sends the number of invalid usernames to the client. 
      The format is int.
   3. If the number of invalid usernames is greater than zero, then the 
      Main Server sends the invalid usernames to the Client in the form of a 
      char[] array. The invalid usernames are separated by spaces.
   4. The Main Server sends the final intersection result to the Client, 
      in the format char[] array. The intersection values are numbers, 
      separated by spaces.

   Messages between Main Server and Servers A,B:
   1. Servers A and B send the number of usernames in a.txt and b.txt 
      respectively to the Main Server. The format is int.
   2. Servers A and B send all the usernames in a.txt and b.txt 
      respectively to the Main Server. The format of the message is char []. 
      The usernames are separated by spaces.
   3. The Main Server sends the number of valid usernames to Servers 
      A and B. The format is int.
   4. If the number of valid usernames is greater than 0, then the Main
      Server sends all the valid usernames to Servers A and B (the valid 
      usernames present in a.txt go to A and those present in b.txt go to B). 
      The valid usernames are separated by spaces.
   5. Then, if the number of valid usernames is greater than 0, 
      the Servers A and B send the intersection of the intervals of their 
      respective valid usernames, in the form of a char[] array. The 
      intersection values are numbers, separated by spaces.


Reused code: 
  In my project, I have utilized some pre-existing code to assist me in my
  implementation. Specifically, I have directly referenced code from 
  Beej's Guide to Network Programming (Pages 34-42).

NOTE: I have used sleep() commands in some places to ensure that 
      send and receive functions are in sync. So there may be a delay of
      a couple of seconds for the results to show up in the terminal. 
