//
// Created by domin on 10/14/2019.
//

#ifndef MYSHELL2_MYSHELL_H
#define MYSHELL2_MYSHELL_H

#endif //MYSHELL2_MYSHELL_H


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

/*GLOBAL VARIABLES*/
int pipe_count=0, fd;
static char* args[512];
char input_buff[1024];
char *exec_cmd[100];
int flag, len;
int flag_pipe=1;
pid_t pid;
int num_Lines;
int envFlag;

int bangFlag;
int pid, status;

char curr_Dir[1000];


char *input_rediFile;
char *output_rediFile;



void clr_var();
void file_Procs ();
void file_Write();
void bangExec();
void environmment();
void set_EnvVariables();
void change_directory();
void parent_directory();
void echo_internal(char *echo_val);

static char* ignore_whiteSpace(char* s);
void tok_cmds(char *com_exec);
void tok_rediIO(char *exec_cmd);
void tok_rediI(char *exec_cmd);
void tok_rediO(char *exec_cmd);
char* ignore_comma(char* str);
static int split(char *exec_cmd, int, int, int);
void piping_exec();
static int command(int, int, int, char *exec_cmd);
void prompt();
void sig_int(int sig_num);

