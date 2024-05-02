/**
 * SimCity™ 3000 Unlimited for modern Linux
 *
 * In July 2000, [Loki Software](https://en.wikipedia.org/wiki/Loki_Entertainment)
 * released a Linux native version of this game.
 * If you try to run the game now, all you will get is `Segmentation fault`
 *
 * But why? The story is interesting :-)
 *
 * Back in 2000 the Linux kernel did not have real support for threading,
 * but it did support processes as schedulable entities through the clone()
 * system call. This call created a copy of the calling process, with
 * the copy sharing the address space of the caller.
 * The LinuxThreads project used this system call to simulate thread support
 * entirely in user space. This approach had numerous disadvantages
 * and that threading model did not conform to POSIX requirements.
 *
 * LinuxThreads was gradualy replaced by NPTL (Native POSIX Thread Library).
 * NPTL requires facilities which were specifically added into the v2.6
 * of the Linux kernel (that version was released in December 2003).
 * LinuxThreads has been removed from glibc since v2.4 (2006).
 *
 * SC3U was, of course, developped and tested only with LinuxThreads in mind.
 * The game relies on 2 "non POSIX compliant" behaviors from LinuxThreads and
 * it segfault if you try to run it with a modern NPTL based glibc.
 *  1. It tries to use pthread_kill() on an invalid thread ID
 *     https://bugzilla.redhat.com/show_bug.cgi?id=169995
 *  2. POSIX dictate that delivering pthread_kill() with SIGSTOP to a thread
 *     will actually stop _all_ threads in that process.
 *     But with LinuxThreads you could stop/resume a particular thread with
 *     SIGCSTOP/SIGCONT. The game use this as a way of doing synchronisation
 *     between threads.
 *
 * The following code is an attempt at fixing those 2 issues and allows you
 * to run this old game on modern systems.    
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int WantDebugLogging;
int (*real_pthread_kill)(pthread_t, int);

void sig_usr_handler(int sig) {
    if (WantDebugLogging)
        fprintf(stderr, "thread[%d] sig_usr_handler(%d)\n", gettid(), sig);

    if (sig == SIGUSR1) {
        int sig_ret;
        sigset_t sigset1;
        sigset_t sigset2;
        sigemptyset(&sigset1);
        sigemptyset(&sigset2);
        sigaddset(&sigset1, SIGUSR1);
        sigaddset(&sigset2, SIGUSR2);
        pthread_sigmask(SIG_BLOCK, &sigset1, NULL);
        if (WantDebugLogging)
            fprintf(stderr, "thread[%d] blocking, waiting for SIGUSR2...\n", gettid());
        sigwait (&sigset2, &sig_ret);
        if (WantDebugLogging)
            fprintf(stderr, "thread[%d] SIGUSR2 recieved, unblocking thread\n", gettid());
        pthread_sigmask(SIG_UNBLOCK, &sigset1, NULL);
    }
}

void __attribute__((constructor)) modernlinuxsetup() {
    WantDebugLogging = 0;
    char *p = getenv("DEBUG");

    if(p != NULL && (strcmp(p, "0") != 0)) {
        WantDebugLogging = 1;
        fprintf(stderr, "sc3u-nptl - hijacking pthread_kill() & ");
        fprintf(stderr, "setting up SIGUSR1 & SIGUSR2 handler\n");
    }
    real_pthread_kill = dlsym(RTLD_NEXT, "pthread_kill");
    signal(SIGUSR1, sig_usr_handler);
    signal(SIGUSR2, sig_usr_handler);
}

int pthread_kill(pthread_t thread, int sig) {
    int ret;

    if (WantDebugLogging)
        fprintf(stderr, "thread[%d] pthread_kill(%d, %d)", gettid(), thread, sig);

    if(thread == 0) {
        ret = 3;  // ESRCH
    } else if (sig == 0) {
        ret = 0;
    } else {
        if (sig == 19) sig = 10; // SIGSTOP -> SIGUSR1
        if (sig == 18) sig = 12; // SIGCONT -> SIGUSR2

        ret = real_pthread_kill(thread, sig);
    }

    if (WantDebugLogging)
        fprintf(stderr, " -> %d \n", ret);

    return ret;
}

