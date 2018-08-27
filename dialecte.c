#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#include "version.h"
#include "leval.h"
#include "lenv.h"

/* Configurable variables */
static char* prompt = "> ";
static char* filenm = NULL;

/* Global dialecte environment. */
static struct lenv* env = NULL;

/** handler_SIGINT exits on Ctrl+C. */
void handler_SIGINT(int sig) {
    (void)sig;
    lenv_free(env);
    fputc('\n', stdout);
    exit(EXIT_SUCCESS);
}

/**
 * Alisp is a small lisp interpreter.
 * http://www.buildyourownlisp.com/contents
 */
int main(int argc, char** argv) {
    signal(SIGINT, handler_SIGINT);

    /* Command line arguments */
    int c;
    while ((c = getopt(argc, argv, "p:f:")) != -1) {
        switch (c) {
        case 'p':
            prompt = optarg;
            break;
        case 'f':
            filenm = optarg;
            break;
        }
    }

    env = lenv_alloc();
    lenv_default(env);

    /* A file is provided, execute it then exit. */
    if (filenm) {
        FILE* file = fopen(filenm, "r");
        if (file) {
            lisp_eval_from_file(env, file);
            lenv_free(env);
            fclose(file);
            return EXIT_SUCCESS;
        } else {
            perror("lisp file opening error");
            return EXIT_FAILURE;
        }
    }

    /* Print infos */
    printf(PROGNAME" "VERSION"-"CODENAME" build %d\n", BUILD);
    puts("Press Ctrl+C to exit.\n");

    size_t prompt_len = strlen(prompt);
    /* REPL loop */
    while (true) {
        char* input = readline(prompt);
        /* Ctrl+D was hit; quit. */
        if (!input || strncmp(input, "quit", 4) == 0) {
            if (!input) {
                fputs("quit\n", stdout);
            }
            free(input);
            break;
        }
        add_history(input);
        lisp_eval_from_string(env, input, prompt_len);
        free(input);
    }

    lenv_free(env);
    return EXIT_SUCCESS;
}
