#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>

#include "version.h"

#define INPUT_SIZE  2048

static char input[INPUT_SIZE];
static char* prompt = "> ";

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
        fputs(prompt, stdout);

        fgets(input, INPUT_SIZE, stdin);

        fputs(input, stdout);
    }

    return EXIT_SUCCESS;
}
