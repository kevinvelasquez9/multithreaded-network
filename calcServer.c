#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"
#include <stdbool.h>
#include <pthread.h>

/* buffer size for reading lines of input from user */
#define LINEBUF_SIZE 1024

struct CalcInfo {
	int clientfd;
    struct Calc *c;
    pthread_mutex_t lock;
};

void fatal(const char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	exit(1);
}

void *worker(void *arg) {
	struct CalcInfo *info = arg;

	/*
	 * set thread as detached, so its resources are automatically
	 * reclaimed when it finishes
	 */
	pthread_detach(pthread_self());

	/* handle client request */
	chat_with_client(info->c, info->clientfd, info->clientfd, &info->lock);
	close(info->clientfd);
	free(info);

	return NULL;
}

int chat_with_client(struct Calc *calc, int infd, int outfd, pthread_mutex_t *lock) {
	rio_t in;
	char linebuf[LINEBUF_SIZE];

	/* wrap standard input (which is file descriptor 0) */
	rio_readinitb(&in, infd);

	/*
	 * Read lines of input, evaluate them as calculator expressions,
	 * and (if evaluation was successful) print the result of each
	 * expression.  Quit when "quit" command is received.
	 */
    while (1) {
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			/* error or end of input */
		} else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			/* quit command */
			return 1;
		} else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0){
            return 0;
        } else {
			/* process input line */
			int result;
            pthread_mutex_lock(lock);
			if (calc_eval(calc, linebuf, &result) == 0) {
				/* expression couldn't be evaluated */
				rio_writen(outfd, "Error\n", 6);
			} else {
				/* output result */
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(outfd, linebuf, len);
				}
			}
            pthread_mutex_unlock(lock);
            
		}
	}
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
		fatal("Usage: ./calcServer <port>");
	}

	int port = atoi(argv[1]);
    if (port < 1024) {
        printf("Not a free port\n");
        return 0;
    }

    struct Calc *calc = calc_create();

	int serverfd = Open_listenfd(argv[1]);
	if (serverfd < 0) {
		fatal("Couldn't open server socket");
	}

	while (1) {
		int clientfd = Accept(serverfd, NULL, NULL);
		if (clientfd < 0) {
			fatal("Error accepting client connection");
		}

		/* create ConnInfo object */
		struct CalcInfo *info = malloc(sizeof(struct CalcInfo));
        pthread_mutex_init(&info->lock, NULL);
		info->clientfd = clientfd;
        info->c = calc;

		/* start new thread to handle client connection */
		pthread_t thr_id;
		if (pthread_create(&thr_id, NULL, worker, info) != 0) {
			fatal("pthread_create failed");
		}
	}
    close(serverfd);
    calc_destroy(calc);
}

