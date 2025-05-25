#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

typedef struct Job {
    // Job properties
    int job_id;
    pid_t pgid;
    char *cmd;

    struct Job *next; // pointer to the next job in the linked list
} Job;

extern Job *jobs_head; // head of the job linked list
extern int next_job_id; // next job id to be assigned

int add_job(pid_t pgid, const char *cmd); // return the added job id

int remove_job(pid_t pgid); // remove the job by its process group ID, return its job id or -1 if not found

void list_jobs(void); // list all jobs in the job list

#endif