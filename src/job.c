#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "job.h"

// initialize extern variables
Job *jobs_head = NULL; 

int add_job(pid_t pgid, const char *cmd, JobState state) {
    int idx = 1;
    while (find_by_jid(idx)) { // find the smallest free job id
        idx++;
    }

    Job *new_job = malloc(sizeof *new_job);
    new_job->job_id = idx;
    new_job->pgid   = pgid;
    new_job->cmd    = strdup(cmd);
    new_job->state  = state;
    new_job->next   = NULL;

    if (!jobs_head) {
        jobs_head = new_job;
    } else {
        Job *t = jobs_head;
        while (t->next) t = t->next;
        t->next = new_job;
    }
    return new_job->job_id;
}

int remove_job(pid_t pgid) {
    // remove in O(n) to iterate through the linked list
    Job *current = jobs_head;
    Job *prev = NULL;

    while (current) {
        if (current->pgid == pgid) { // find the job with the given pgid
            int job_id = current->job_id;

            if (prev) {
                prev->next = current->next; // remove
            } else {
                jobs_head = current->next; // when the job is at the head, prev is empty
            }

            free(current->cmd);
            free(current);
            return job_id; // return the job id of the removed job
        }

        prev = current;
        current = current->next;
    }

    return -1; // job not found
}

void list_jobs(void) {
    int max_id = 0;
    for (Job *j = jobs_head; j; j = j->next) {
        if (j->job_id > max_id) max_id = j->job_id;
    }
    if (max_id == 0) return;  // no jobs

    for (int id = 1; id <= max_id; id++) {
        Job *job = find_by_jid(id);
        if (job) {
            const char *state = (job->state == RUNNING) ? "Running" : "Stopped";
            printf("[%d] %s\t%s%s\n", job->job_id, state, job->cmd, (job->state == RUNNING) ? " &" : ""); // append '&' if the job is running
        }
    }
}

Job *find_by_pgid(pid_t pgid) {
    Job *current = jobs_head;
    while (current) {
        if (current->pgid == pgid) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

Job *find_by_jid(int job_id) {
    Job *current = jobs_head;
    while (current) {
        if (current->job_id == job_id) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

