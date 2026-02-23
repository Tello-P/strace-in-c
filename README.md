# MyStrace: x86_64 System Call Tracer

MyStrace is a diagnostic utility for Linux that monitors and logs system calls performed by a process. By utilizing the `ptrace` API, the tool intercepts the execution flow to extract syscall identifiers, arguments, and return values directly from the CPU registers.

## Key Features
- **Process Attachment**: Can spawn a new process or attach to an existing one using a PID (`-p` / `--pid`).
- **Register Inspection**: Accesses the `user_regs_struct` to decode x86_64 system calls.
- **Memory Peeking**: Implements a custom data reader to extract strings and buffers from the tracee's memory space.
- **Error Handling**: Detects and reports standard Linux error codes (errno) returned by the kernel.

## Technical Architecture
The application operates as a tracer that manages a tracee through the following mechanism:
1. **Synchronization**: Uses `waitpid` to synchronize with the tracee's state changes.
2. **State Management**: Implements a state-machine logic to differentiate between syscall entry (where arguments are present) and syscall exit (where the return value is available).
3. **Data Extraction**: Uses `PTRACE_PEEKDATA` to traverse the tracee's memory when system calls involve pointers (e.g., file paths or buffers).

## Installation

You can install MyStrace directly to your system path using the provided installation script:

```bash
chmod +x install.sh
./install.sh

