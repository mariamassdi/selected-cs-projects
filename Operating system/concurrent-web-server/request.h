#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "log.h"
#include <sys/time.h>

int readers_inside, writers_inside, writers_waiting;
double DS_time; 
pthread_cond_t read_allowed;
pthread_cond_t write_allowed;
pthread_mutex_t global_lock;

typedef struct Threads_stats {
    int id;           // Thread ID
    int stat_req;     // Number of static requests handled
    int dynm_req;     // Number of dynamic requests handled
    int post_req;     // Number of POST requests handled
    int total_req;    // Total number of requests handled
} * threads_stats;

typedef struct request
{
	int fd;
	struct timeval arrival;
	struct timeval dispatch;

    struct timeval log_enter;
    struct timeval log_exit;
	threads_stats thread_stats;
} request;

// Handles a client request.
// - fd: the connection socket
// - arrival: time the request arrived
// - dispatch: time the thread began processing the request
// - t_stats: pointer to the current thread's statistics (must be updated by student)
// - log: server-wide shared log (thread-safe access required)

// - must correctly track and update per-thread statistics inside the request handler.
// - Update the following fields in `threads_stats`:
//   - total_req
//   - stat_req (for static requests)
//   - dynm_req (for dynamic requests)
//   - post_req (for POST requests)
// - These values should reflect accurate request processing for each thread and be used in response headers/logs.

void requestHandle(request *request, server_log log);

#endif
