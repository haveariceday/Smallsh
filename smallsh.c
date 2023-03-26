//Author: Hiromi Watanabe
//Date: Feb 11, 2023
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdint.h>

//functions
//char *prompt(char *buffer);
//char * str_gsub(char *restrict *restrict haystack, char const *restrict needle, char const *restrict sub);

//Helper function for Expansion
char * str_gsub(char *restrict *restrict haystack, char const *restrict needle, char const *restrict sub)
{
	char *str = *haystack;
	size_t haystack_len = strlen(str);
	size_t const needle_len = strlen(needle),
	             sub_len = strlen(sub);
	
	for (; (str = strstr(str, needle));) {
		ptrdiff_t off = str - *haystack;
		if(sub_len > needle_len){
			str = realloc(*haystack, sizeof **haystack * (haystack_len + sub_len - needle_len + 1));
			if (!str) goto exit;
			*haystack = str;
			str = *haystack + off;
     }
/*
	haystack = "this is my haystack!!!"
	needle = "stacl"
	sub = "heap"
*/
	memmove(str + sub_len, str + needle_len, haystack_len + 1 - off - needle_len);
	memcpy(str, sub, sub_len);
	haystack_len = haystack_len + sub_len - needle_len;
	str += sub_len;
     if(strcmp(needle,"~")==0){
      break;
    }
	}
	str = *haystack;
	if (sub_len < needle_len) {
		str = realloc(*haystack, sizeof **haystack * (haystack_len + 1));
		if (!str)goto exit;
		*haystack = str;
	}

exit:
	return str;
}

//Global variable
char *exitStatus = "0";
char *processId = "";
bool commentFlag = false;
bool andFlag = false;
int exit_status = 0;
int stat = 0;
int p_id = 0;

int main(){
  while(1==1){
  fflush(stdin);
  fflush(stdout);
  while((p_id = waitpid(0, &stat, WUNTRACED | WNOHANG))>0){
    if(WIFEXITED(stat)){
      fprintf(stderr, "Child process %jd done. Exit status %d.\n",(intmax_t) p_id, WEXITSTATUS(stat));
    }
    if(WIFSIGNALED(stat)){
      fprintf(stderr, "Child process %jd done. Signaled %d.\n", (intmax_t) p_id, WTERMSIG(stat));
    }
    if(WIFSTOPPED(stat)){
      kill(p_id,SIGCONT);
      fprintf(stderr, "Child process %jd stopped. Continuing.\n", (intmax_t) p_id);
    }
  }
  char *buffer=NULL;
  char *ps=getenv("PS1");
  if(ps==NULL){
    ps = "";
  }

  size_t size;
  ssize_t read;

  printf("\n%s",ps);
  read = getline(&buffer, &size, stdin);
  if(read < 0){
    puts("Couldn't read the input");
    clearerr(stdin);
    break;
  }
  if(feof(stdin)!=0){
    fprintf(stderr,"\nexit\n");
    clearerr(stdin);
    kill(0,SIGINT);
  }

  //word splitting
  int i = 0;
  char *ifs=getenv("IFS");
  char *token;
  char *tokens[513];
  if(ifs==NULL){
    ifs=" \t\n";
  }
  token = strtok(buffer, ifs);
  while(token!=NULL){
    tokens[i] = strdup(token);
    //printf(" %s\n", tokens[i]); //check
    token = strtok(NULL, ifs);
    i+=1;
  }

  // Expansion
  char *home=getenv("HOME");
  if(home==NULL){
	  home="";
  }
  for(int j=0; j<i; j++){
    //~/ expansion
	  if(strstr(tokens[j],"~/")){
		  str_gsub(&tokens[j], "~", home);
	  }
    //$$ expansion
    int mypid = getpid();
    char *pid=malloc(sizeof(int));
    sprintf(pid, "%d", mypid);
    str_gsub(&tokens[j], "$$", pid);
    //$? expansion exit status
    str_gsub(&tokens[j], "$?", exitStatus);
    //$! expansion
    str_gsub(&tokens[j], "$!", processId);
    //printf("\nafter expansion: %s", tokens[j]);  //check
    free(pid);
  }
  //Parsing
  char *infile=NULL;
  char *outfile=NULL;
  for(int k=0; k<i; k++){
    if(commentFlag){
       free(tokens[k]);
       tokens[k]=NULL;
    }
    else if(strstr(tokens[k], "#")){
       commentFlag=true;
       if(tokens[k-1] && strstr(tokens[k-1], "&")){
          andFlag=true;
          free(tokens[k]);
          tokens[k]=NULL;
       }       
       free(tokens[k]);
       tokens[k]=NULL;
    }
    else if(strstr(tokens[k], "&")){
        andFlag=true;
        free(tokens[k]);
        tokens[k]=NULL;
    }
    else if(strstr(tokens[k],"<")){
       infile = tokens[k+1];       
       free(tokens[k]);
       tokens[k]=NULL;
	    //printf("\ninfilename: %s", infile); //check
    }
    else if(strstr(tokens[k], ">")){
       outfile = tokens[k+1];
       free(tokens[k]);
       tokens[k]=NULL;
       //printf("\noutfilename: %s", outfile); //check
    }
    //printf("\nafter parsing: %s", tokens[k]); //check
  }

  //Create a big command list
  char *commandList[513];
  for(int n=0; n<i+1; n++){
    if(tokens[n]==NULL){
      commandList[n] = NULL;
      break;
    }
    else{
      commandList[n] = tokens[n];
    }
    //printf("\ncommand list: %s, andflag: %d, commentflag: %d", commandList[n], andFlag, commentFlag); //check having freeing them will mess up command list
  }

    // built-in exit
    if(commandList[0] && strcmp(commandList[0],"exit")==0){
       if(!commandList[1]){
          kill(0,SIGINT);
       }
       else if(commandList[2]){
         fprintf(stderr,"Too many arguments provided for exit");
       }
       else if(commandList[1] && isdigit(*commandList[1])){
         exit_status = atoi(commandList[1]);
         fprintf(stderr, "\nexit\n");
         exit(exit_status);
         kill(0,SIGINT);
       }
       else if(commandList[1] && !isdigit(*commandList[1])){
         fprintf(stderr,"Incorrect argument provided for exit");
       }
      free(buffer);
      buffer=NULL;
      free(token);
      free(*tokens);
      *tokens=NULL;
      memset(&tokens[0],0,sizeof(tokens));
      memset(&commandList[0],0,sizeof(commandList));
      commentFlag = false;
      andFlag = false;
      continue;
    }
  
      // built-in cd
     if(commandList[0] && strcmp(commandList[0],"cd")==0){
       if(commandList[2]){
         fprintf(stderr,"Too many arguments provided for cd");
       }
       else if(commandList[1]){
         int res = chdir(commandList[1]);
         if(res == -1){
         perror("Failed chdir()");
         }
       }
       else{
         char *home = getenv("HOME");
         chdir(home);
       }
      free(buffer);
      buffer=NULL;
      free(token);
      free(*tokens);
      *tokens=NULL;
      memset(&tokens[0],0,sizeof(tokens));
      memset(&commandList[0],0,sizeof(commandList));
      commentFlag = false;
      andFlag = false;
      continue;
      }


  //Exec
  int childStatus = 0;
  pid_t spawnPid = fork();
  switch(spawnPid){
    case -1:
      perror("fork()\n");
      exit(1);
      break;
    case 0:
      //printf("\nCHILD(%d) running command\n", getpid());
      if(infile!=NULL){
        int inputFile = open(&infile[0], O_RDONLY);
        if(inputFile==-1){
          perror("source open()");
          exit(1);
        }
        int result = dup2(inputFile,0);
        if(result ==-1){
          perror("source dup2()");
          exit(2);
        }
      }
      if(outfile!=NULL){
        int outputFile = open(outfile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
        if(outputFile==-1){
          perror("target open()");
          exit(1);
        }
        int result = dup2(outputFile, 1);
        if(result==-1){
          perror("target dup2()");
          exit(2);
        }
      }
      //printf("first: %s ... the rest: %s, %s",commandList[0], commandList[1], commandList[2]);
      execvp(commandList[0], commandList);
      perror("execvp");
      exit(2);
      break;
    default:
	    if(!andFlag){ //foreground
        waitpid(spawnPid, &childStatus,0);
        exitStatus=malloc(sizeof(int));
        sprintf(exitStatus, "%d", WEXITSTATUS(childStatus));
      }
      else{//background
        processId=malloc(sizeof(int));
        sprintf(processId, "%d", spawnPid);
      }
      //printf("\nPARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
		  //exit(0);
		    break;
  }
//loopEnd:
  //exitStatus=malloc(sizeof(int));
  //sprintf(exitStatus, "%d", childStatus);
  //printf("\nexit status: %s",exitStatus);

  //processId=malloc(sizeof(int));
  //sprintf(processId, "%d", spawnPid);
  //printf("\nprocess id: %s", processId);*/


  //Free
  free(buffer);
  buffer=NULL;
  free(token);
  ///token=NULL;
  //free(ifs);
  //free(ps);
  //free(home);
  //free(infile);
  //free(outfile);
  free(*tokens);
  *tokens=NULL;
  //free(exitStatus);
  //free(processId);
  //memset(&buffer[0],0,sizeof(buffer));
  memset(&tokens[0],0,sizeof(tokens));
  memset(&commandList[0],0,sizeof(commandList));
  commentFlag = false;
  andFlag = false;
  }
  return 0;
}
