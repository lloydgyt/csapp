/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>
 */
#include <assert.h>
#include <bits/types/sigset_t.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Misc manifest constants */
#define MAXLINE 1024   /* max line size */
#define MAXARGS 128    /* max args on a command line */
#define MAXJOBS 16     /* max jobs at any point in time */
#define MAXJID 1 << 16 /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;   /* defined in libc */
char prompt[] = "tsh> "; /* command line prompt (DO NOT CHANGE) */
int verbose = 0;         /* if true, print additional output */
int nextjid = 1;         /* next job ID to allocate */
char sbuf[MAXLINE];      /* for composing sprintf messages */

struct job_t {             /* The job struct */
    pid_t pid;             /* job PID */
    int jid;               /* job ID [1, 2, ...] */
    int state;             /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE]; /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
static volatile sig_atomic_t fg_flag = 0;
static sigset_t mask, previous_mask;
/* End global variables */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
pid_t Fork();
int Execve(const char *pathname, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *status, int options);
int Kill(pid_t pid, int sig);
int Setpgid(pid_t pid, pid_t pgid);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv) {
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h': /* print help message */
            usage();
            break;
        case 'v': /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':            /* don't print a prompt */
            emit_prompt = 0; /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT, sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler); /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler); /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);
    /* Init signal mask */
    Sigfillset(&mask); // TODO should I block all?

    /* Execute the shell's read/eval loop */
    while (1) {

        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        // TODO fgets read a string from stdin to cmdline?
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        // TODO what does this do? detect eof?
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
 */
void eval(char *cmdline) {
    int bg;
    // TODO number of argv? this might cause a problem
    int num_args = 10;
    char **argv = (char **)malloc(num_args * sizeof(*argv));
    pid_t pid;
    // 1. parse the line to get part
    // assume parseline cut any extra space
    bg = parseline(cmdline, argv);
    if (bg == -1) {
        return; /* ignore blank line */
    }
    // 2. get first word as command and test if builtin

    Sigprocmask(SIG_SETMASK, &mask, &previous_mask);
    if (!builtin_cmd(argv)) {
        /* Not built-in*/
        // printf("cmd: %s\n", argv[0]);
        // printf("first arg: %s\n", argv[1]);
        // TODO need to consider blocking signals later!
        if ((pid = Fork()) == 0) {
            /* CHILD */
            // set pgid here, to separate the child from tsh!
            Sigprocmask(SIG_SETMASK, &previous_mask, NULL);
            Setpgid(0, 0);
            // TODO do I need to add env?
            Execve(argv[0], argv, NULL);
        }
        /* PARENT child never reaches here! */
        assert(pid > 0);
        if (bg) {
            /* background */
            addjob(jobs, pid, BG, cmdline);
            // TODO unblock SIGCHLD here!
            // print a line of message
            struct job_t *jobp = getjobpid(jobs, pid);
            printf("[%d] (%d) %s\n", jobp->jid, jobp->pid, jobp->cmdline);
        } else {
            /* fg */
            addjob(jobs, pid, FG, cmdline);
            waitfg(pid); // handle fgjob exited, terminated or stopped
        }
    }
    Sigprocmask(SIG_SETMASK, &previous_mask, NULL);
    return;
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv) {
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf) - 1] = ' ';   /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    } else {
        delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        } else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0) /* ignore blank line */
        return -1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }
    return bg;
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv) {
    char *cmd = argv[0];
    assert(cmd);
    if (strcmp(cmd, "quit") == 0) {
        exit(0);
    } else if (strcmp(cmd, "jobs") == 0) {
        // TODO should I protect this??
        listjobs(jobs);
        return 1;
    } else if (strcmp(cmd, "bg") == 0 || strcmp(cmd, "fg") == 0) {
        do_bgfg(argv);
        return 1;
    }
    return 0; /* not a builtin command */
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) {
    char *cmd = argv[0];
    char *id = argv[1];
    bool bg = (strcmp(cmd, "bg") == 0);
    if (!id) {
        printf("%s command requires PID or %%jobid argument\n", cmd);
        return;
    }
    // handle 2 types of ID to get the JID
    pid_t stopped_pid;
    int stopped_jid;
    struct job_t *stopped_job;

    bool is_jid = (*id == '%') ? true : false;
    if (is_jid) {
        id += 1;
    }
    char *end;
    long num = strtol(id, &end, 10);
    if (*end != '\0' || num < 0 || num > INT_MAX) {
        printf("%s: argument must be a PID or %%jobid\n", cmd);
        return;
    }
    stopped_jid = is_jid ? (int)num : pid2jid((int)num);
    stopped_job = getjobjid(jobs, stopped_jid);
    if (!stopped_job) {
        if (is_jid) {
            printf("%%%d: No such job\n", stopped_jid);
        } else {
            printf("(%d): No such process\n", (int)num);
        }
        return;
    }
    stopped_pid = stopped_job->pid;

    // then it runs in the bg/fg
    // TODO send CONT to a group or singal process
    Kill(-stopped_pid, SIGCONT);
    stopped_job->state = bg ? BG : FG;
    if (bg) {
        printf("[%d] (%d) %s\n", stopped_job->jid, stopped_job->pid,
               stopped_job->cmdline);
    } else {
        waitfg(stopped_pid);
    }
    return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {
    // make sure when at entry point, it's blocked
    Sigprocmask(SIG_SETMASK, &previous_mask, NULL);
    while (!fg_flag) {
        sleep(1);
    }
    // when it reaches here, fg job is either terminated or stopped
    struct job_t *job = getjobpid(jobs, pid);
    assert((!job) || job->state == ST);
    fg_flag = 0; // reset flag!
    Sigprocmask(SIG_SETMASK, &mask, &previous_mask);
    return;
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig) {
    int old_errno = errno;
    pid_t child_pid;
    int status;
    // SIGCHLD is not queued! here, we should set up a loop!
    while ((child_pid = waitpid(-1, &status, WUNTRACED | WNOHANG)) > 0) {
        int jid = pid2jid(child_pid);
        assert(jid);
        struct job_t *job = getjobpid(jobs, child_pid);
        // check for fg first! then set flag
        if (job->state == FG) {
            fg_flag = 1;
        }
        // check status
        if (WIFEXITED(status)) {
            deletejob(jobs, child_pid);
        } else if (WIFSIGNALED(status)) {
            printf("Job [%d] (%d) terminated by signal %d\n", jid, child_pid,
                   WTERMSIG(status));
            deletejob(jobs, child_pid);
        } else if (WIFSTOPPED(status)) {
            printf("Job [%d] (%d) stopped by signal %d\n", jid, child_pid,
                   WSTOPSIG(status));
            job->state = ST;
        }
    }
    errno = old_errno;
    return;
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig) {
    int olderrno = errno;
    pid_t foreground_pid = fgpid(jobs);
    if (foreground_pid) {
        Kill(-foreground_pid, SIGINT);
    }
    // send SIGINT to that group, don't handle cleanup!
    errno = olderrno;
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig) {
    int olderrno = errno;
    pid_t foreground_pid = fgpid(jobs);
    if (foreground_pid) {
        printf("Catch TSTP\n");
        Kill(-foreground_pid, SIGTSTP);
    }
    // send SIGINT to that group, don't handle cleanup!
    errno = olderrno;
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) {
    int i, max = 0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) {
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if (verbose) {
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid,
                       jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs) + 1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) {
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) {
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
            case BG:
                printf("Running ");
                break;
            case FG:
                printf("Foreground ");
                break;
            case ST:
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ", i,
                       jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) {
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg) {
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg) {
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}

pid_t Fork() {
    pid_t pid;

    if ((pid = fork()) < 0) {
        unix_error("Fork error!");
    }
    return pid;
}

int Execve(const char *filename, char *const argv[], char *const envp[]) {
    int ret = execve(filename, argv, envp);
    if (ret < 0) {
        printf("%s: Command not found\n", filename);
        exit(EXIT_FAILURE); // Typically won't reach here if execve succeeds
    }
    /* never reaches here */
    return ret;
}

pid_t Wait(int *statusp) {
    pid_t pid = wait(statusp);
    if (pid < 0) {
        perror("wait error");
        exit(EXIT_FAILURE);
    }
    return pid;
}

// TODO don't use this!
pid_t Waitpid(pid_t pid, int *status, int options) {
    pid_t ret = waitpid(pid, status, options);
    if (ret < 0) {
        perror("waitpid error");
        exit(EXIT_FAILURE);
    }
    return ret;
}
/**
 * Wrapper for kill()
 * Sends a signal to a process or process group.
 * On failure, prints error and exits.
 */
int Kill(pid_t pid, int sig) {
    if (kill(pid, sig) == -1) {
        fprintf(stderr, "Kill error: %s (pid: %d, sig: %d)\n", strerror(errno),
                pid, sig);
        exit(EXIT_FAILURE);
    }
    return 0;
}

/**
 * Wrapper for setpgid()
 * Sets the process group ID for the given PID.
 * On failure, prints an error and exits.
 */
int Setpgid(pid_t pid, pid_t pgid) {
    if (setpgid(pid, pgid) == -1) {
        fprintf(stderr, "Setpgid error: %s (pid: %d, pgid: %d)\n",
                strerror(errno), pid, pgid);
        exit(EXIT_FAILURE);
    }
    return 0;
}
void Sigemptyset(sigset_t *set) {
    if (sigemptyset(set) == -1) {
        perror("sigemptyset error");
        exit(EXIT_FAILURE);
    }
}

void Sigfillset(sigset_t *set) {
    if (sigfillset(set) == -1) {
        perror("sigfillset error");
        exit(EXIT_FAILURE);
    }
}

void Sigaddset(sigset_t *set, int signum) {
    if (sigaddset(set, signum) == -1) {
        perror("sigaddset error");
        exit(EXIT_FAILURE);
    }
}

void Sigdelset(sigset_t *set, int signum) {
    if (sigdelset(set, signum) == -1) {
        perror("sigdelset error");
        exit(EXIT_FAILURE);
    }
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (sigprocmask(how, set, oldset) == -1) {
        perror("sigprocmask error");
        exit(EXIT_FAILURE);
    }
}
