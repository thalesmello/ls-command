/*
Implementation of the ls command
Thales Mello
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX 100

int pathnum;
char *defaultpath = "/bin:.";
char ** path;

int strtoarray(char *str, char *sep, char *** list);
/* --> Separates str into an array of string, spliting by occurances of sep */
void freestrarray(char ** list, int size);
/* --> Free array of strings allocated dynamically */
void printstrarray(char **list, int size);
/* --> Used for debugging. Prints list of array */
void getPath();
/* --> Converts string THEPATH into a list of individual pathes */
void readline(char *input);
/* --> Reads entire line from stdin */
int isexecutable(char *filepath);
/* --> Verifies is a file at a given path is executable */
int findfilepath(char *cmd, char *filepath);
/* --> Iterates over list of paths in order to find executable file whose name is cmd */
int posstrinarray(char * str, char ** list, int size);
/* --> Returns position of a occurance of str in a list of strings. Returns -1 if not found */
void execfilepath(int argslen, char * cmd, char ** args, char * filepath);
/* --> Executes cmd at filepath */
void redirfd(int redirfd, char *pathname, int flags, mode_t mode);
/* --> Applies dup2 to given redirfd and file descriptor returned by pathname */
void child(int argslen, char * cmd, char ** args, char * filepath);
/* --> Function that defines behavior of a newly born child process */
int verifyampersand(char ** inputlist, int size);
/* --> Returns 1 if & is at end of input list, -1 if at another position, or 0 if not found */
void executecommand(char *cmd, char **args, int argslen, char *filepath);
/* --> Executes operations given a certain command */
void handlecommand(char *cmd, char **args, int argslen);
/* --> Helper function for main function */

int main()
{
  printf("welcome to myshell\n");
  getPath();
  char input[MAX];
  char **inputlist;
  int size;
  while(1){
    printf("---| ");
    readline(input);
    size = strtoarray(input, " ", &inputlist);
    handlecommand(inputlist[0], inputlist+1, size-1);
    freestrarray(inputlist, size);
  }
  freestrarray(path, pathnum);
  return 0;
}
int strtoarray(char *str, char *sep, char *** list)
{
  int len = strlen(str);
  char *copystr = (char *) malloc((len + 1) * sizeof(char));
  strcpy(copystr, str);
  int sizelist = 0;
  char *cur, *temp;
  temp = copystr;
  while( ( cur = strsep(&temp, sep) ) )
  {
    sizelist++;
  }
  (*list) = (char **) malloc(sizelist * sizeof(char*));
  int i = 0;
  int curlen;
  strcpy(copystr, str);
  temp = copystr;
  while( ( cur = strsep(&temp, sep) ) )
  {
    curlen = strlen(cur);
    (*list)[i] = (char *) malloc((curlen + 1) * sizeof(char));
    strcpy((*list)[i], cur);
    i++;
  }
  free(copystr);
  return sizelist;
}
void freestrarray(char ** list, int size)
{
  int i;
  for(i = 0; i<size; i++)
  {
    free(list[i]);
  }
  free(list);
}
void printstrarray(char **list, int size)
{
  int i;
  for(i = 0; i<size; i++)
  {
    printf("%s\n",list[i]);
  }
}
void getPath()
{
  char *mypath = getenv("THEPATH");
  if(mypath == NULL)
    mypath = defaultpath;
  printf("($THEPATH) is %s\n",mypath);
  pathnum = strtoarray(mypath, ":", &path);
}
void readline(char *input)
{
  scanf("%[^\n]", input);
  getchar();
}
int isexecutable(char *filepath)
{
  struct stat buf;
  int rc = lstat(filepath, &buf );
  if(rc < 0)
    return 0;
  if ( S_ISREG( buf.st_mode ) )
  {
    if ( buf.st_mode & ( S_IXUSR | S_IXGRP | S_IXOTH ) )
    {
      return 1;
    }
  }

  return 0;
}
int findfilepath(char *cmd, char *filepath)
{
  int i;
  char cur[MAX];
  for(i=0; i<pathnum; i++)
  {
    strcpy(cur,path[i]);
    strcat(cur,"/");
    strcat(cur,cmd);
    if(isexecutable(cur)){
      strcpy(filepath,cur);
      return 1;
    }
  }
  return 0;
}
int posstrinarray(char * str, char ** list, int size)
{
  int i;
  for(i = 0; i<size; i++)
  {
    if( strcmp(list[i],str)==0 )
      return i;
  }
  return -1;
}
void execfilepath(int argslen, char * cmd, char ** args, char * filepath)
{
  switch(argslen){
    case 0:
      execlp(filepath, cmd, NULL); break;
    case 1:
      execlp(filepath, cmd, args[0], NULL); break;
    case 2:
      execlp(filepath, cmd, args[0], args[1], NULL); break;
    case 3:
      execlp(filepath, cmd, args[0], args[1], args[2], NULL); break;
    case 4:
      execlp(filepath, cmd, args[0], args[1], args[2], args[3], NULL); break;
  }
  perror( "execlp() failed!" );
  exit(EXIT_FAILURE);
}
void redirfd(int redirfd, char *pathname, int flags, mode_t mode)
{
  int fd;
  if(mode == -1){
    fd = open(pathname, flags);
  }
  else{
    fd = open(pathname, flags , mode);
  }
  if( fd < 0 )
  {
    perror("ERROR: Failed to operate file\n");
    exit(-1);
  }
  if((dup2(fd, redirfd) == -1))
  {
    perror("ERROR: Failed to redirect file descriptor\n");
    exit(-1);
  }
}
void child(int argslen, char * cmd, char ** args, char * filepath)
{
  int pos, cmdlen;
  cmdlen = argslen;
  if( (pos = posstrinarray("==>", args, argslen)) != -1 )
  {
    cmdlen = pos;
    redirfd(1, args[pos+1], O_WRONLY | O_CREAT | O_TRUNC , 0664);
  }
  else if( (pos = posstrinarray("<==", args, argslen)) != -1 )
  {
    cmdlen = pos;
    redirfd(0, args[pos+1], O_RDONLY, -1);
  }
  else if( (pos = posstrinarray("-->", args, argslen)) != -1 )
  {
    cmdlen = pos;
    redirfd(1, args[pos+1], O_WRONLY | O_CREAT | O_APPEND , 0664);
  }
  execfilepath(cmdlen, cmd, args, filepath);
}
int verifyampersand(char ** inputlist, int size){
  int i;
  for(i = 0; i< size-1; i++){
    if(strcmp(inputlist[i], "&")==0){
      return -1;
    }
  }
  if(strcmp(inputlist[size-1], "&")==0)
    return 1;
  return 0;
}
void executecommand(char *cmd, char **args, int argslen, char *filepath)
{
  int ampersand = verifyampersand(args, argslen);
  if(ampersand == -1){
    printf("ERROR: & Must be at the end of command\n");
  }
  int pid = fork();
  if(pid < 0){
    perror( "fork() failed!" );
    exit(EXIT_FAILURE);
  }
  else if( pid == 0 ){
    if(ampersand == 1)
      child(argslen-1, cmd, args, filepath);
    else
      child(argslen, cmd, args, filepath);
  }
  else{
    if(ampersand != 1){
      int status;
      wait( &status );
    }
    else{
      printf("[process running in background with pid %d]\n",pid);
    }
  }
}
void handlecommand(char *cmd, char **args, int argslen)
{
  char filepath[MAX];
  if(strcmp(cmd,"cd")==0){
    if(chdir(args[0])!=0){
      printf("cd: %s: No such file or directory\n", cmd);
    }
  }
  else if(findfilepath(cmd, filepath)){
    executecommand(cmd, args, argslen, filepath);
  }
  else if( (strcmp(cmd, "quit")==0 || strcmp(cmd, "exit")==0) && argslen == 0){
    printf("bye\n");
    exit(EXIT_SUCCESS);
  }
  else{
    printf("ERROR: command '%s' not found.\n", cmd);
  }
}
