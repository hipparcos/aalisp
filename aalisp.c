#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "version.h"
#include "lisp.h"

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

    /* REPL loop */
    while (true) {
        char* input = readline(prompt);
        add_history(input);
        lisp_eval_from_string(input);
        free(input);
    }

    return EXIT_SUCCESS;
}
