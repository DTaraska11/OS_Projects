Linux Shell Implementation 






This program simulates a Linux shell command line interpreter. 
A command line interpreter is a program that allows the entering of commands and then executes those commands to the operating system. 






This implementation supports: 


- I/O redirection 
Including ‘<’, ‘>’, or ‘>>’ will redirect input or output from/to the specified file. For example, 
echo hello world! >test.txt
Will redirect the output of ‘echo hello world!’ to the text file test.txt. If the file does not exist, it will be created. 




<(inputfile) - takes input from “inputfile”
>(outputfile)- outputs to ‘outputfile’, file is created if DNE and truncated if it does
>>(outputfile)- same as above but file is appended if it exists 


-  Piping
        Extension of redirection. Allows output of one program to be passed as input to           
        another. 
- Internal Commands: 
        
cd <directory> - changes current directory, if no argument is passed then it reports the current directory
help - displays help information
clr - clears the screen 
dir <directory>- displays contents of the desired directory 
quit - quits the shell
env - lists all the environment strings 
echo <comment> - displays <comment> followed by a new line
pause - pause operation of the shell until <enter> is pressed


-Background execution 
        Including a ‘&’ at the end of the command line indicates that the shell  
        should return to the prompt immediately after launching a program 


-Batchfile
        Invoking the shell with an argument containing a file will start it in 
        batch mode. 
        The shell will accept and process commands in the batch file, instead 
        of taking user input like in ‘interactive mode’.


-Program invocation 
        Apart from internal commands, other commands are interpreted as 
        program 
        invocation. For example, the command ./test.exe will execute test as 
        it’s own seperate process.