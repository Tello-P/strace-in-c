#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>



int main(){


  pid_t child;

  /* CHILD PROCESS */
  if ((child = fork()) == 0){
    printf("hello from the child\n");
    printf("This is the PID: %d\n", getpid());
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);  // PID, ADDR and DATA are ignored if TRACEME 
    char *args[] = {"/bin/ls", "-l", "/home", NULL};  // REMOVE THIS WITH REAL ARGS
    
    // execvp(program route, args)
    execvp(args[0], args);

    perror("execvp error");
    exit(EXIT_FAILURE);
}
  else if(child<0){
    perror("fork error");
    return -1;
  }
  /* FATHER PROCESS */
  else{
    printf("hello from the parent\n");
    printf("This is the PID: %d\n", getpid());
    
    int state;

    wait(&state);

    while (WIFSTOPPED(state)){
      printf("Child is stopped\n");
      return 1;
    }

  }


  return 0;
}
