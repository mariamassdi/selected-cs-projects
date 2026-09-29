#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>        
#include "log.h"
#include "queue.h"
#include <unistd.h>

/*
 * append_stats2: כותב סטטיסטיקות לתוך buf ומחזיר כמה תווים נוספו.
 */
int append_stats2(char* buf,
                  threads_stats t_stats,
                  struct timeval arrival,
                  struct timeval dispatch,
                  struct timeval log_enter,
                  struct timeval log_exit)
{
    int offset = 0;

    offset += sprintf(buf + offset, "Stat-Req-Arrival:: %ld.%06ld\r\n",
                      arrival.tv_sec, arrival.tv_usec);

    offset += sprintf(buf + offset, "Stat-Req-Dispatch:: %ld.%06ld\r\n",
                      dispatch.tv_sec, dispatch.tv_usec);

    offset += sprintf(buf + offset, "Stat-Log-Arrival:: %ld.%06ld\r\n",
                      log_enter.tv_sec, log_enter.tv_usec);

    offset += sprintf(buf + offset, "Stat-Log-Dispatch:: %ld.%06ld\r\n",
                      log_exit.tv_sec, log_exit.tv_usec);

    offset += sprintf(buf + offset, "Stat-Thread-Id:: %d\r\n",
                      t_stats->id);

    offset += sprintf(buf + offset, "Stat-Thread-Count:: %d\r\n",
                      t_stats->total_req);

    offset += sprintf(buf + offset, "Stat-Thread-Static:: %d\r\n",
                      t_stats->stat_req);

    offset += sprintf(buf + offset, "Stat-Thread-Dynamic:: %d\r\n",
                      t_stats->dynm_req);

    offset += sprintf(buf + offset, "Stat-Thread-Post:: %d\r\n\r\n",
                      t_stats->post_req);

    return offset;
}

// Opaque struct definition
struct Server_Log {
    queue log_entries; 
};

request* copy_request(request* original)
{
    if (!original) return NULL;

    request* new_req = malloc(sizeof(request));
    if (!new_req) return NULL;

    // Copy by value
    new_req->fd = original->fd;
    new_req->arrival = original->arrival;
    new_req->dispatch = original->dispatch;
    new_req->log_enter = original->log_enter;
    new_req->log_exit = original->log_exit;

    // Deep copy of thread_stats
    if (original->thread_stats) {
        threads_stats new_stats = malloc(sizeof(struct Threads_stats));
        if (!new_stats) {
            free(new_req);
            return NULL;
        }
        memcpy(new_stats, original->thread_stats, sizeof(struct Threads_stats));
        new_req->thread_stats = new_stats;
    } else {
        new_req->thread_stats = NULL;
    }

    return new_req;
}

// Creates a new server log instance
server_log create_log(double DS)
{
    readers_inside = 0;
    writers_inside = 0;
    writers_waiting = 0;
    DS_time=DS;
    pthread_cond_init(&read_allowed, NULL);
    pthread_cond_init(&write_allowed, NULL);
    pthread_mutex_init(&global_lock, NULL);

    server_log new_log = (server_log)malloc(sizeof(struct Server_Log));
    init(&new_log->log_entries);
    return new_log;
}

void destroy_log(server_log log)
{
    if (log != NULL) {
        Node* current = log->log_entries.head;
        while (current != NULL) {
            request* req = current->request;
            if (req) {
                free(req->thread_stats);
                free(req);
            }
            current = current->next;
        }

        destroy(&log->log_entries);
        free(log);
    }

    pthread_cond_destroy(&read_allowed);
    pthread_cond_destroy(&write_allowed);
    pthread_mutex_destroy(&global_lock);
}


int get_log(server_log log, char** dst)
{
    // Reader entry protocol

 


  
    const size_t PER_ENTRY = 700;
    size_t cap = (size_t)log->log_entries.size * PER_ENTRY + 1;

    *dst = (char*)malloc(cap);
    if (*dst == NULL) {
        return -1;
    }

    int len = 0;


    (*dst)[0] = '\0';

    for (Node* current = log->log_entries.head; current != NULL; current = current->next) {
        
        len += append_stats2(*dst + len,
                             current->request->thread_stats,
                             current->request->arrival,
                             current->request->dispatch,
                             current->request->log_enter,
                             current->request->log_exit);
        if (current->next != NULL)
        {
            (*dst)[len++] = '#';
            (*dst)[len] = '\0';
        }
        // אופציונלי: אם את רוצה ממש להגן מפני overflow:
        // if ((size_t)len >= cap) { ... }  (אבל cap גדול מספיק בדרך כלל)
    }
    

    // Reader exit protocol

    return len;
}

/*
 * add_to_log: מוסיף רשומה ללוג ומחזיר גם log_enter/log_exit של פעולת הכתיבה.
 */
void add_to_log(server_log log, request* req)
{
    // Writer entry protocol


    // FIX #7: "first thing in critical area" — למדוד מיד אחרי שההמתנה נגמרה והכניסה אושרה.


    request* copy = copy_request(req);

    // FIX #8: חובה לבדוק copy לפני שמדליקים עליו שדות (מניעת segfault)
    if (copy) {
        // FIX #9: לשמור על הרשומה בלוג את זמני הכניסה/יציאה ללוג (לא של req עצמו)
        copy->log_enter = req->log_enter;
        copy->log_exit  = req->log_exit;

        push_back(&log->log_entries, copy);
    }

    // Writer exit protocol
   
}
void writer_lock()
{
	pthread_mutex_lock(&global_lock);
    writers_waiting++;
   
    while (readers_inside > 0 || writers_inside > 0)
    	pthread_cond_wait(&write_allowed, &global_lock);

    writers_waiting--;
    writers_inside++;
  
    pthread_mutex_unlock(&global_lock);
}

void writer_unlock()
{
	 pthread_mutex_lock(&global_lock);
   	 writers_inside--;
	if(writers_inside==0)
	{
    if (writers_waiting > 0)
        pthread_cond_signal(&write_allowed);
    else
        pthread_cond_broadcast(&read_allowed);
	}
    pthread_mutex_unlock(&global_lock);
}
void reader_lock()
{
	
    pthread_mutex_lock(&global_lock);
    while (writers_inside > 0 || writers_waiting > 0)
        pthread_cond_wait(&read_allowed, &global_lock);


    readers_inside++;
   
    pthread_mutex_unlock(&global_lock);

  
}
void reader_unlock()
{
	    pthread_mutex_lock(&global_lock);
    readers_inside--;
    if (readers_inside == 0)
        pthread_cond_signal(&write_allowed);
    pthread_mutex_unlock(&global_lock);

}