# Technical Internals and Development Log

## System Call Interception Architecture

MyStrace is built on the `ptrace(2)` API, which allows one process (the tracer) to observe and control the execution of another process (the tracee). On the `x86_64` architecture, system calls are triggered by the `syscall` instruction, causing a transition from user mode to kernel mode.

### The Entry/Exit Cycle

The tracer utilizes `PTRACE_SYSCALL`, which causes the kernel to stop the tracee at two specific points for every system call:

1. **Syscall Entry**: Just before the kernel executes the service. At this point, `orig_rax` contains the syscall number and other registers hold the input arguments.
2. **Syscall Exit**: Just after the kernel completes the service. At this point, `rax` contains the return value or error code.

The implementation manages this cycle using a state-machine logic (an `entry` toggle) to ensure data is extracted and displayed at the correct execution phase.

---

## Process Synchronization and Reliability

A critical challenge during development was ensuring the tracer and tracee remained perfectly synchronized.

### Resolved Challenge: Race Conditions in Register Access

Initially, the control loop attempted to read registers immediately after signaling the tracee to continue. This led to "stale" data or duplicates because the tracer was sampling registers before the tracee had actually reached the next trap.

**Solution:** The loop was refactored to enforce a strict synchronization sequence:

1. **Resume**: Invoke `PTRACE_SYSCALL`.
2. **Wait**: Block execution using `waitpid(child_pid, &state, 0)`.
3. **Validate**: Verify the process state using `WIFEXITED`.
4. **Sample**: Only after a successful wait is `PTRACE_GETREGS` called to read the `user_regs_struct`.


[Watch Demo Video](getRegsProblem.mp4)

This ensures the tracer always reads fresh register values provided by the kernel at the moment of the stop.

---

## Memory Inspection (`read_data`)

Because system call arguments often pass pointers to memory buffers (e.g., file paths in `openat` or data in `write`), reading registers alone is insufficient.

The tool implements a custom memory reader using `PTRACE_PEEKDATA`:

* **Word-Aligned Reads**: Since `ptrace` reads memory in words (8 bytes on `x86_64`), the function fetches data in chunks.
* **Buffer Reconstruction**: The reader iterates through these chunks, casting them to `char*` and detecting null-terminators (`\0`) to stop at the end of strings.
* **Safety**: The implementation currently uses a 4KB buffer, which covers the standard `PATH_MAX` limit in Linux.

---

## Attachment Logic: Fork vs. Attach

The project supports two distinct methods of process control, each with specific kernel requirements.

### Tracee Spawning (PTRACE_TRACEME)

When launching a new program, the child process calls `PTRACE_TRACEME` followed by `execvp`. The kernel stops the process at the first instruction of the new executable, allowing the tracer to set options like `PTRACE_O_TRACESYSGOOD` before any syscalls occur.

### Running Process Attachment (PTRACE_ATTACH)

Implementing the `-p/--pid` flag introduced different requirements:

* **Signal Handling**: `PTRACE_ATTACH` sends a `SIGSTOP` to the target. The tracer must acknowledge this stop via `waitpid` before it can begin the `PTRACE_SYSCALL` loop.
* **Permissions**: Attachment is subject to Linux Security Modules (LSM) such as Yama. The implementation assumes the user has the necessary `CAP_SYS_PTRACE` capabilities or that `ptrace_scope` allows for the attachment.
* **Cleanup**: Unlike the fork method, attached processes require an explicit `PTRACE_DETACH` to resume their original execution state without the tracer.

---

## Architecture Mapping (x86_64)

The implementation adheres to the System V AMD64 ABI for register mapping:

* **Syscall ID**: `orig_rax`
* **Arguments**: `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9`
* **Return Value**: `rax` (Values between -1 and -4095 are treated as error codes).

---

