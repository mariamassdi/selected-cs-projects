#include "segel.h"
#include "request.h"
#include "log.h"
#include "queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
typedef struct udp_request
{
    struct sockaddr_in client_addr;
    int udpfd;
    struct udp_request *next;
} udp_request;

typedef struct
{
    int thread_id;
    server_log log;
    struct Threads_stats t_stats;
    udp_request *udp_head;
    udp_request *udp_tail;

} thread_arguments;

pthread_mutex_t lock;
pthread_cond_t worker_thread_cond;
pthread_cond_t buffer_available_cond;

queue waiting_Regular_queue;
queue working_queue;

int num_of_threads;
int num_of_requests;
double debug_sleep_time;
int the_pipe[2];
void block(request *request);

void block(request *request)
{
    while (waiting_Regular_queue.size + working_queue.size >= num_of_requests)
    {
        pthread_cond_wait(&buffer_available_cond, &lock);
    }
    gettimeofday(&(request->arrival), NULL);
    push_back(&waiting_Regular_queue, request);
}

static long Pint(const char *str, const char *argument_name)
{
    char *end_ptr;
    long value;

    if (str == NULL || str[0] == '\0')
    {
        fprintf(stderr, "Invalid %s\n", argument_name);
        exit(1);
    }

    errno = 0;
    value = strtol(str, &end_ptr, 10);

    if (errno == ERANGE || end_ptr == str || *end_ptr != '\0')
    {
        fprintf(stderr, "Invalid %s: %s\n",
                argument_name, str);
        exit(1);
    }

    return value;
}

static double Pdouble(const char *str,
                      const char *argument_name)
{
    char *end_ptr;
    double value;

    if (str == NULL || str[0] == '\0')
    {
        fprintf(stderr, "Invalid %s\n", argument_name);
        exit(1);
    }

    errno = 0;
    value = strtod(str, &end_ptr);

    if (errno == ERANGE || end_ptr == str || *end_ptr != '\0')
    {
        fprintf(stderr, "Invalid %s: %s\n",
                argument_name, str);
        exit(1);
    }

    return value;
}
// Parses command-line arguments

void getargs(int *tcp_port,int *udp_port, int *num_threads, int *num_requests, double *debug_sleep_time, int argc,char *argv[])
{
    //check them all as asked in piazza
    if (argc != 6)
    {
        fprintf(stderr, "Usage: %s <tcp_port> <udp_port> <threads> "
                        "<queue_size> <debug_sleep_time>\n",argv[0]);
        exit(1);
    }

    long p1 = Pint(argv[1], "TCP port");

    long p2 = Pint(argv[2], "UDP port");
    long p3 = Pint(argv[3], "number of threads");
    long p4 = Pint(argv[4], "queue size");
    *debug_sleep_time = Pdouble(argv[5], "debug sleep time");

    if (p1 < 1024 || p1 > 65535)
    {
        fprintf(stderr, "Invalid TCP port\n");

        exit(1);
    }

    if (p2 < 1024 || p2 > 65535)
    {
        fprintf(stderr, "Invalid UDP port\n");

        exit(1);
    }

    if (p1 == p2)
    {
        fprintf(stderr, "TCP and UDP ports must be different\n");
        exit(1);
    }

    if (p3 <= 0)
    {
        fprintf(stderr, "Invalid number of threads\n");
        exit(1);
    }

    if (p4 <= 0)
    {
        fprintf(stderr, "Invalid queue size\n");
        exit(1);
    }
    *tcp_port = (int)p1;

    *udp_port = (int)p2;
    *num_threads = (int)p3;
    *num_requests = (int)p4;
}

void *worker_thread_main(void *arg)
{
    thread_arguments *args = (thread_arguments *)arg;
    // struct Threads_stats t_stats;
    // t_stats.id = args->thread_id;
    // t_stats.stat_req = 0;
    // t_stats.dynm_req = 0;
    // t_stats.total_req = 0;
    // t_stats.post_req = 0;

    while (1)
    {
        pthread_mutex_lock(&lock);

        while (waiting_Regular_queue.size == 0 && args->udp_head == NULL)
        {
            pthread_cond_wait(&worker_thread_cond, &lock);
        }

        if (args->udp_head != NULL)
        {
            udp_request *udp = args->udp_head;

            args->udp_head = udp->next;

            if (args->udp_head == NULL)
            {
                args->udp_tail = NULL;
            }

            pthread_mutex_unlock(&lock);

            char udp_response[MAXLINE];

            int response_length = snprintf(
                udp_response,
                MAXLINE,
                "Stat-Thread-Id:: %d\n"
                "Stat-Thread-Count:: %d\n"
                "Stat-Thread-Static:: %d\n"
                "Stat-Thread-Dynamic:: %d\n"
                "Stat-Thread-Post:: %d\n",
                args->t_stats.id,
                args->t_stats.total_req,
                args->t_stats.stat_req,
                args->t_stats.dynm_req,
                args->t_stats.post_req);

            if (UDP_Write(udp->udpfd, &udp->client_addr, udp_response, response_length) < 0)
            {
                unix_error("UDP write error");
            }

            free(udp);

            continue;
        }
        // Dequeue the regular request
        struct request *request = get_front(&waiting_Regular_queue);
        pop_front(&waiting_Regular_queue);
        request->thread_stats = &args->t_stats;
        // Add the request to the working queue
        push_back(&working_queue, request);
        gettimeofday(&(request->dispatch), NULL);

        // Unlock the mutex, allow other threads to work
        pthread_mutex_unlock(&lock);

        // Handle the request
        requestHandle(request, args->log);
        Close(request->fd);

        pthread_mutex_lock(&lock);
        remove_by_request(&working_queue, request); ///////////// we may need this
        // Remove the request from the working queue

        // Signal the main thread if needed
        pthread_cond_signal(&buffer_available_cond);

        pthread_mutex_unlock(&lock);
        char notify = '1';
        ssize_t writeR;

        do
        {
            writeR = write(the_pipe[1],
                           &notify,
                           sizeof(notify));
        } while (writeR < 0 && errno == EINTR);

        if (writeR < 0 &&
            errno != EAGAIN &&
            errno != EWOULDBLOCK)
        {
            unix_error("pipe write error");
        }
        free(request);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int listenfd, udpfd, connfd, tcp_port, udp_port;
    socklen_t clientlen;

    struct sockaddr_in clientaddr;
    struct sockaddr_in udp_clientaddr;
    getargs(&tcp_port, &udp_port, &num_of_threads, &num_of_requests, &debug_sleep_time, argc, argv); // Create the global server log
    server_log log = create_log(debug_sleep_time);

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&worker_thread_cond, NULL);
    pthread_cond_init(&buffer_available_cond, NULL);

    init(&waiting_Regular_queue);
    init(&working_queue);
    if (pipe(the_pipe) < 0)
    {
        unix_error("pipe error");
    }

    for (int i = 0; i < 2; i++)
    {
        int flags = fcntl(the_pipe[i], F_GETFL);

        if (flags < 0 ||
            fcntl(the_pipe[i],
                  F_SETFL,
                  flags | O_NONBLOCK) < 0)
        {
            unix_error("fcntl error");
        }
    }
    pthread_t *threads = malloc(sizeof(pthread_t) * (num_of_threads));

    if (threads == NULL)
    {
        fprintf(stderr, "Failed to allocate threads array\n");
        exit(1);
    }

    thread_arguments *args = malloc(sizeof(thread_arguments) * (num_of_threads));

    for (int i = 0; i < num_of_threads; i++)
    {
        args[i].thread_id = i + 1;
        args[i].log = log;
        args[i].t_stats.id = i + 1;
        args[i].t_stats.stat_req = 0;
        args[i].t_stats.dynm_req = 0;
        args[i].t_stats.post_req = 0;
        args[i].t_stats.total_req = 0;
        args[i].udp_head = NULL;
        args[i].udp_tail = NULL;
        if (pthread_create(&threads[i], NULL, worker_thread_main, (void *)&args[i]) != 0)
        {
            free(threads);
            free(args);
            fprintf(stderr, "Failed to create worker thread %d\n", i + 1);
            exit(1);
        }
    }

    listenfd = Open_listenfd(tcp_port);
    udpfd = UDP_Open(udp_port);
    while (1)
    {
        int isPlace;

        pthread_mutex_lock(&lock);

        isPlace = waiting_Regular_queue.size + working_queue.size < num_of_requests;

        pthread_mutex_unlock(&lock);
        fd_set readfds;
        FD_ZERO(&readfds);

        FD_SET(udpfd, &readfds);
        FD_SET(the_pipe[0], &readfds);

        if (isPlace)
        {
            FD_SET(listenfd, &readfds);
        }
        int maxfd = udpfd;

        if (the_pipe[0] > maxfd)
        {
            maxfd = the_pipe[0];
        }

        if (isPlace && listenfd > maxfd)
        {
            maxfd = listenfd;
        }
        Select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(the_pipe[0], &readfds))
        {
            char pipe_buffer[64];
            ssize_t bytes_read;

            while (1)
            {
                bytes_read = read(the_pipe[0],
                                  pipe_buffer,
                                  sizeof(pipe_buffer));

                if (bytes_read > 0)
                {
                    continue;
                }

                if (bytes_read < 0 && errno == EINTR)
                {
                    continue;
                }

                if (bytes_read < 0 &&errno != EAGAIN &&errno != EWOULDBLOCK)
                {
                    unix_error("pipe read error");
                }

                break;
            }
        }
        if (FD_ISSET(udpfd, &readfds))
        {
            char udp_buffer[MAXLINE];
            struct sockaddr_in udp_clientaddr;

            int bytes_read = UDP_Read(udpfd, &udp_clientaddr, udp_buffer, MAXLINE - 1);
            if (bytes_read < 0)
            {
                unix_error("UDP read error");
            }

            if (bytes_read > 0)
            {
                udp_buffer[bytes_read] = '\0';
                char *end_ptr;
                errno = 0;

                long parsed_thread_id = strtol(udp_buffer, &end_ptr, 10);

                if (errno == 0 && end_ptr != udp_buffer && *end_ptr == '\0' && parsed_thread_id >= 1 && parsed_thread_id <= num_of_threads)
                {
                    int thread_id = (int)parsed_thread_id;
                    udp_request *new_udp = malloc(sizeof(udp_request));

                    if (new_udp == NULL)
                    {
                        perror("malloc");
                        exit(1);
                    }

                    new_udp->client_addr = udp_clientaddr;
                    new_udp->udpfd = udpfd;
                    new_udp->next = NULL;

                    pthread_mutex_lock(&lock);

                    if (args[thread_id - 1].udp_tail == NULL)
                    {
                        args[thread_id - 1].udp_head = new_udp;
                        args[thread_id - 1].udp_tail = new_udp;
                    }
                    else
                    {
                        args[thread_id - 1].udp_tail->next = new_udp;
                        args[thread_id - 1].udp_tail = new_udp;
                    }

                    pthread_cond_broadcast(&worker_thread_cond);

                    pthread_mutex_unlock(&lock);
                }
            }
        }

        if (isPlace && FD_ISSET(listenfd, &readfds))
        {
            struct request *request = malloc(sizeof(struct request));
            request->log_enter.tv_sec = 0;
            request->log_enter.tv_usec = 0;
            request->log_exit.tv_sec = 0;
            request->log_exit.tv_usec = 0;
            clientlen = sizeof(clientaddr);
            connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

            request->fd = connfd;
            pthread_mutex_lock(&lock);

            // Regular request logic
            // if (waiting_Regular_queue.size + working_queue.size >= num_of_requests)
            // {
            //     block(request); // Call with regular queue logic
            // }
            // else
            // {
            gettimeofday(&(request->arrival), NULL);
            push_back(&waiting_Regular_queue, request);
            // }
            pthread_cond_signal(&worker_thread_cond); // Wake up worker thread
            pthread_mutex_unlock(&lock);
        }
    }

    // Cleanup threads
    for (int i = 0; i < num_of_threads; i++)
    {
        pthread_cancel(threads[i]);
        pthread_join(threads[i], NULL);
    }

    free(args);
    free(threads);
    Close(udpfd);
    Close(listenfd);
    // Destroy pthread objects
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&worker_thread_cond);
    pthread_cond_destroy(&buffer_available_cond);

    // Clean up the server log before exiting
    destroy_log(log);
    destroy(&waiting_Regular_queue);
    destroy(&working_queue);
    return 0;
}
