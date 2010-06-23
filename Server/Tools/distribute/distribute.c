/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * A small program to distribute load among computers.
 *
 * Syntax:
 *   distribute host1 host2 host3 smphost smphost < worklist.txt
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMDLINE 10000

#define MAX_VERBOSE 3

#define EXIT_INTERNAL_ERROR 1
#define EXIT_SCRIPT_FAILED  2
#define EXIT_BAD_PARAMS     3


struct worker {
   char *hname;
   int pid;
};

struct worker *workers;
int num_workers;
int num_activated;
int verbose=1;
int die_on_error=0;
int num_started=0;
int num_errors=0;


void wait_one(int ignore_error);


// Called in the child
void start_target(const char *worker_name, const char *cmdline)
{
   char *cmdbuf;
   char *insertpos;

   cmdbuf = malloc(strlen(cmdline)*2 + strlen(worker_name) + 100);
   sprintf(cmdbuf, "spawn -noecho ssh -x -t %s \"", worker_name);
   insertpos=cmdbuf + strlen(cmdbuf);
   while (*cmdline) {
      switch (*cmdline) {
         case '"':
         case '\\':
         case '[':
         case ']':
         case '$':
            *(insertpos++) = '\\';
            *(insertpos++) = *cmdline;
            break;
         default:
            *(insertpos++) = *cmdline;
            break;
      }
      ++cmdline;
   }
   *insertpos=0;
   strcat(cmdbuf, "\"");
   if (verbose > 2) {
      fprintf(stderr, "Executing expect -c set timeout -1 -c %s "
                      "-c expect { foomatic exp_continue } "
                      "-c set resultcode [wait]; exit [lindex $resultcode 3]; \n", cmdbuf);
   } else if (verbose > 1) {
      fprintf(stderr, "  Starting %s\n", cmdbuf);
   }
   execlp("expect", "expect", "-c", "set timeout -1", 
                              "-c", cmdbuf, 
                              "-c", "expect { foomatic exp_continue }",
                              "-c", "set resultcode [wait]; exit [lindex $resultcode 3]; ",
          NULL);
   perror("exec failed:");
   exit(EXIT_INTERNAL_ERROR);
}

void start_worker(int worker_id, const char *cmdline)
{
   int newpid;

   newpid = fork();
   if (newpid<0) {
      perror("Fork failed:");
      exit(EXIT_INTERNAL_ERROR);
   } else if (newpid==0) {
      // The child
      start_target(workers[worker_id].hname, cmdline);
   } else {
      // The parent
      workers[worker_id].pid = newpid;
      if (verbose > 2) {
         fprintf(stderr, "Forked new pid=%i\n", newpid);
      }
      ++num_started;
   }
}

void stop_all(void)
{
   int i;

   for (i=0; i<num_workers ; i++) {
      if (workers[i].pid != 0) {
         kill(workers[i].pid, SIGHUP);
      }
   }
   while (num_activated > 0) {
      wait_one(1);
   }
}

void worker_returned_nonzero(int i, int status, int ignore_error)
{
   fprintf(stderr, 
         "Worker %i (%s) returned non-zero result %i : %s.\n", 
         i, workers[i].hname, status,
         ignore_error?"Ignoring":"Terminating" );
   ++num_errors;
   if (!ignore_error) {
      --num_activated;  /* This worker is already dead */
      stop_all();
      exit(EXIT_SCRIPT_FAILED);
   }
}


int find_worker(void)
{
   int i;
   pid_t donepid;
   int status;

   if (num_activated < num_workers) {
      /* Still filling the workers */
      if (verbose > 2) {
         fprintf(stderr, "Activating worker %i\n", num_workers);
      }
      return num_activated++;
   } else {
      /* All have been activated, wait for one of them */
      donepid = wait(&status);
      for (i=0; i<num_workers; i++) {
         if (workers[i].pid == donepid) {
            workers[i].pid = 0;
            if (status != 0) {
               worker_returned_nonzero(i, status, !die_on_error);
            }
            if (verbose > 2) {
               fprintf(stderr, "Reusing worker %i\n", i);
            }
            return i;
         }
      }
      fprintf(stderr, "Unexpected pid returned from wait: %i\n", donepid);
      exit(EXIT_INTERNAL_ERROR);
   }
}


int start_one(void)
{
   char cmdline[MAX_CMDLINE];
   char *retval;
   char *lastchar;
   int worker_id;

   retval=fgets(cmdline, sizeof(cmdline), stdin);
   if (retval == NULL) {
      if (verbose > 2) {
         fprintf(stderr, "start_one: no more work to do\n");
      }
      return 0;
   }

   /* Strip cr/lf from end */
   lastchar = cmdline + (strlen(cmdline) - 1);
   while ( (lastchar >= cmdline) && 
           ( (*lastchar=='\r') || (*lastchar=='\n') ) ) {
      *lastchar=0;
      --lastchar;
   }
   if (verbose > 2) {
      fprintf(stderr, "start_one: pruned cmd is: [%s]\n", cmdline);
   }

   worker_id = find_worker();
   if (verbose > 2) {
      fprintf(stderr, "start_one: got free worker: %i\n", worker_id);
   } else if (verbose > 0) {
      fprintf(stderr, "Starting on %i (%s) : %s\n", 
            worker_id, workers[worker_id].hname, cmdline);
   }

   start_worker(worker_id, cmdline);

   return 1;
}

void wait_one(int ignore_error)
{
   int i;
   pid_t donepid;
   int status;

   donepid = wait(&status);
   for (i=0; i<num_workers; i++) {
      if (workers[i].pid == donepid) {
         workers[i].pid = 0;
         if (status != 0) {
            worker_returned_nonzero(i, status, ignore_error);
         }
         if (verbose > 1) {
            fprintf(stderr, "Worker %i (%s) is done\n", i, workers[i].hname);
         }
         --num_activated;
         return;
      }
   }
   fprintf(stderr, "Unexpected pid returned from wait: %i\n", donepid);
   exit(EXIT_INTERNAL_ERROR);
}

void usage(void)
{
   printf("\n"
        "distribute - distributes task among computers\n"
        "\n"
        "  Usage:\n"
        "     distribute [-h] [-v]* [-s]* host host ....\n"
        "        -h   show these usage instructions\n"
        "        -v   increase verbosity level (may be repeated) (default %i)\n"
        "        -s   decrease verbosity level (may be repeated)\n"
        "        -e   die on error (default is to ignore errors)\n"
        "\n"
        "     Lines of work to perform are read from stdin and distributed\n"
        "     to the hosts given in the list of hosts. Each time a host \n"
        "     finished one item it receives the next item on the list. Once\n"
        "     the list is emptied this program waits until the last items \n"
        "     are compleated\n"
        "\n"
        "     Multiprocessor hosts can be listed twice in the list of hosts\n"
        "\n",
        verbose);
}

int main(int argc, char *argv[])
{
   int max_workers;
   int i;

   max_workers = argc-1;
   workers = malloc(sizeof(struct worker[max_workers]));

   num_workers=0;
   for (i=1; i<argc ; i++) {
      if (argv[i][0] == '-') {
         // Handle options
         switch (argv[i][1]) {
            case 'v':
               ++verbose;
               if (verbose > MAX_VERBOSE) verbose=MAX_VERBOSE;
               break;
            case 's':
               --verbose;
               if (verbose < 0) verbose=0;
               break;
            case 'e':
               die_on_error = 1;
               break;
            case 'h':
               usage();
               exit(0);
            default:
               fprintf(stderr, "Unrecognized option (-%c).\n", argv[i][1]);
               exit(EXIT_BAD_PARAMS);
         }
      } else {
         workers[num_workers].hname = argv[i];
         workers[num_workers].pid = 0;
         ++num_workers;
      }
   }
   if (num_workers <= 0) {
      fprintf(stderr, "No workers specified, aborting\n");
      usage();
      exit(EXIT_BAD_PARAMS);
   }

   if (verbose > 0) {
      fprintf(stderr, "Verbosity set to %i\n", verbose);
   }
   if (verbose > 1) {
      fprintf(stderr, "Got %i workers:\n", num_workers);
      for (i=0; i<num_workers; i++) {
         fprintf(stderr, "   %s\n", workers[i].hname);
      }
   }

   /* No workers running yet */
   num_activated = 0;
   while (start_one()) { }

   // All started, let them finish and wait for them
   if (verbose > 0) {
      fprintf(stderr, "All tasks started, waiting for the last ones to finish\n");
   }
   while (num_activated > 0) {
      wait_one(!die_on_error);
   }

   if (verbose > 0) {
      fprintf(stderr, "Started %i workers, %i returned with an error\n",
            num_started, num_errors);
   }
   return 0;
}

