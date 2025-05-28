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

int add_job(pid_t pgid, const char *cmd); // return the added job id

int remove_job(pid_t pgid); // remove the job by its process group ID, return its job id or -1 if not found

void list_jobs(void); // list all jobs in the job list

Job *find_by_pgid(pid_t pgid); // for sigchld handler to find the job by its process group ID

Job *find_by_jid(int job_id); // for add_job to add to the smallest free job id

#endif