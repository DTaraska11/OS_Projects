Dominic Taraska Tug76525
CIS 3207 Project 3 Networked Spell Checker

Takes either 0, 1, or 2 arguments. One specifies the file to use as dictionary file and other specifies desired port number. 
If using gcc to compile, make sure to include -pthread, such as:


gcc -o server main.c open_listenfd.c utility.c server.h -pthread 


Once you run the program, it will automatically act as a server. To connect to the server as a host, you use: 


telnet localhost [PORT NUMBER] 


Once connected, you can start sending words to be spell checked. 

Default port number is 1198, and default dictionary file is titled "dictionary.txt"


I have also uploaded testing screenshots as well as screenshots of the program in use as well as the log file.

