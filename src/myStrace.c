#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
 #include <sys/user.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "../include/syscall_names_x86_64.h"



char* read_data(pid_t pid, unsigned long regAddr){
  int allocated=4096;
  char *data = malloc(allocated);  // no particular reason, just might be enough
  long buffer;
  int bytesRegistered=0;

  while (1){
    buffer = ptrace(PTRACE_PEEKDATA, pid, regAddr+bytesRegistered, NULL); //8 bytes each time
    if (buffer==-1.0){
      printf("\nWARNING: ptrace_peekdata returned -1\n");
      break;
    }
    
    char *aux = (char *)&buffer; // convert to char

    for (int i=0; i<8; i++){
      data[bytesRegistered] = aux[i];
      bytesRegistered++;
      if (aux[i] == '\0'){
        return data;
      }
    }
  }
    return data;
}


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
    int entry=1;// ALWAYS START AT ONE
    char *syscallName;

    waitpid(child_pid, &state, 0);
  

    struct user_regs_struct regs; //struct to get regs defined in user.h  
    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD); // better syscall tracking
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
    
      if (entry) {
            syscallName = get_syscall_name(regs.orig_rax);
            // SYSCALL ENTRY: orig_rax is the ID
            printf("Syscall: %-15s (ID: %3llu) ",
                   syscallName, regs.orig_rax);
            entry = 0; 
        } else {
            // SYSCALL EXIT: rax is the Result
            if ((long long)regs.rax < 0 && (long long)regs.rax > -4096) {
                printf(" | Result: ERROR (%lld)\n", (long long)regs.rax);
            } else {
                printf(" | Result: 0x%llx\n", regs.rax);
                // returning the data
                if (strcmp(syscallName, "openat")==0){
                char *dataString = read_data(child_pid, regs.rsi);
                printf("DATA: %s\n", dataString);
          }
            }
            entry = 1;
        }


    }
     
    }



  return 0;
}
