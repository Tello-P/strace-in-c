#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
 #include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "../include/syscall_names_x86_64.h"


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


    struct user_regs_struct regs; //struct to get regs defined in user.h
    while (1){

      if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0)==-1){
        perror("ptrace exited");
        break;
      }  // Makes the tracee advance till the next stop
      /////////////////////////////////////////////////////////////
      waitpid(child_pid, &state, 0);
      if (WIFEXITED(state)){
        printf("Process exited\n");
        return 1;
      }
      /////////////////////////////////////////////////////////////
 

      if (ptrace(PTRACE_GETREGS, child_pid, NULL, &regs) == -1) {
        perror("ptrace(PTRACE_GETREGS)");
        break;
      }
      
      printf("Syscall num: %llu\n", regs.orig_rax);
      printf("Syscall def: %s\n", get_syscall_name(regs.orig_rax));

     
    }

  }


  return 0;
}
