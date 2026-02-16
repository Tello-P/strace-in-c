#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>



int main(){


  pid_t child_pid;

  /* CHILD PROCESS */
  if ((child_pid = fork()) == 0){
    printf("hello from the child\n");
    printf("This is the PID: %d\n", getpid());
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);  // PID, ADDR and DATA are ignored if TRACEME 
    char *args[] = {"/bin/ls", "-l", "/home", NULL};  // REMOVE THIS WITH REAL ARGS
    
    // execvp(program route, args)
    execvp(args[0], args);

    perror("execvp error");
    exit(EXIT_FAILURE);
}
  else if(child_pid<0){
    perror("fork error");
    return -1;
  }
  /* FATHER PROCESS */
  else{
    printf("hello from the parent\n");
    printf("This is the PID: %d\n", getpid());
    
    int state;

    waitpid(child_pid, &state, 0);


    while (1){

      if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0)==-1){
        perror("ptrace exited");
        break;
      }  // Makes the tracee advance till the next stop


      waitpid(child_pid, &state, 0);
      if (WIFEXITED(state)){
        printf("Process exited\n");
        break;
      }

      printf("Child with PID: %d stopped\n", child_pid);

    }

  }


  return 0;
}
