#include <stdio.h>
#include <stdlib.h>

#include "version.h"

#define MACRO_TO_STR(s) #s
#define STR(s) MACRO_TO_STR(s)

int main(int argc, char** argv) {
    /* Print infos */
    puts(PROGNAME" "VERSION"-"CODENAME" build "STR(BUILD));
    puts("Press Ctrl+C to exit.\n");

    return EXIT_SUCCESS;
}
