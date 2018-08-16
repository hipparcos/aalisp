#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "version.h"
#include "leval.h"
#include "lenv.h"

/* Configurable variables */
static char* prompt = "> ";

/**
 * Alisp is a small lisp interpreter.
 * http://www.buildyourownlisp.com/contents
 */
int main(int argc, char** argv) {
    /* Command line arguments */
    int c;
    while ((c = getopt(argc, argv, "p:")) != -1) {
        switch (c) {
        case 'p':
            prompt = optarg;
            break;
        }
    }

    /* Print infos */
    printf(PROGNAME" "VERSION"-"CODENAME" build %d\n", BUILD);
    puts("Press Ctrl+C to exit.\n");

    struct lenv* env = lenv_alloc();
    lenv_default(env);

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
