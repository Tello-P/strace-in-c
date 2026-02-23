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


int main(int argc, char* argv[]){

  pid_t child_pid;
  int isPidSelected=0;

  for (int i=0; i< argc; i++){
    if ((strcmp(argv[i], "-p")==0) || (strcmp(argv[i], "--pid")==0)){
      isPidSelected=1;
      child_pid = atoi(argv[i+1]);
      printf("pid: %d\n", child_pid);
      break;
    }
    else if ((strcmp(argv[i], "-h")==0)||(strcmp(argv[i], "--help")==0)){
      printf("HELP MENU\n");
      printf("Usage: \n\t mystrace (path to command) (args of command)\n\t mystrace (-p/--pid) (pid of process)\n");
      exit(1);
    }
  }

  /* CHILD PROCESS */
  if (!isPidSelected && (child_pid = fork()) == 0){ // if returns 0 is the child, else is the father
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);  // PID, ADDR and DATA are ignored if TRACEME 

    // execvp(program route, args)
    execvp(argv[1], argv);

    perror("execvp error");
    exit(EXIT_FAILURE);
  }
  else if(!isPidSelected && child_pid<0){
    perror("fork error");
    return -1;
  }
  /* FATHER PROCESS */
  else{
    int state;
    int entry=1;// ALWAYS START AT ONE
    char *syscallName;

    if (isPidSelected && ptrace(PTRACE_ATTACH, child_pid, 0, 0)==-1){
      perror("ptrace attach exited");
      exit(-1);
    }

    waitpid(child_pid, &state, 0);

    struct user_regs_struct regs; //struct to get regs defined in user.h  
    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD); // better syscall tracking
    while (1){


      if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0)==-1){
        perror("ptrace exited");
        break;
      }  // Makes the tracee advance till the next stop

      waitpid(child_pid, &state, 0);
      if (WIFEXITED(state)){
        printf("Process exited\n");
        return 1;
      }

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
          if (strcmp(syscallName, "openat")==0 || (strcmp(syscallName, "write")==0) || strcmp(syscallName, "read")==0){
            char *dataString = read_data(child_pid, regs.rsi);
            printf("\n---------------\n");
            printf("DATA: %s\n", dataString);
            printf("-----------------\n");
          }
        }
        entry = 1;
      }
    }
    ptrace(PTRACE_DETACH, child_pid, NULL, NULL);
  }
  return 0;
}
