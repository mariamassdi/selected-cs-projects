
# Operating Systems

Selected academic projects developed as part of the Operating Systems course at the Technion – Israel Institute of Technology.

These projects focus on low-level programming, Linux system calls, process management, concurrent programming, networking, and dynamic memory management using C and C++.

## Projects

### 1. Mini Linux Shell

[View Source Code](mini-linux-shell)

Implementation of a Unix-like command-line shell in C++.

**Technical highlights:**

- Built-in and external command execution.
- Process creation and management.
- Foreground and background processes.
- Job control and signal handling.
- Pipes and output redirection.
- Linux system calls: fork, exec, waitpid, kill, and dup2.
- Object-oriented design and command abstraction.
- System call error handling.

### 2. Concurrent Web Server

[View Source Code](concurrent-web-server)

Implementation of a multithreaded web server in C, focusing on concurrent request handling, networking, and thread synchronization.

**Technical highlights:**

- Thread pool architecture.
- POSIX threads (pthreads).
- TCP and UDP socket programming.
- Mutexes and condition variables.
- Bounded request queues.
- Concurrent client request handling.
- I/O multiplexing using select.
- Request logging and synchronization.
- Shared resource management.

### 3. Dynamic Memory Allocator

[View Source Code](memory-allocator)

Implementation of custom memory allocation mechanisms in C++, progressing from basic heap allocation to a buddy-based memory allocator.

**Technical highlights:**

- Custom memory allocation functions: smalloc, scalloc, sfree, and srealloc.
- Heap management using sbrk.
- Memory mapping using mmap and munmap.
- Buddy memory allocation algorithm.
- Memory block splitting and merging.
- Free block management.
- Memory metadata and allocation statistics.
- Pointer arithmetic and bitwise operations.

## Technologies and Skills

- C and C++
- Linux and POSIX APIs
- Low-Level Programming
- System Calls
- Process and Thread Management
- Multithreading and Synchronization
- TCP/UDP Socket Programming
- Dynamic Memory Management
- Object-Oriented Programming
- Git and Makefiles

## Academic Context

Developed as part of the B.Sc. in Computer Science at the Technion.
