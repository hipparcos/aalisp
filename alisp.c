#include <stdio.h>
#include <stdlib.h>

#define STR(s) #s
#define XSTR(s) STR(s)

int main(int argc, char** argv) {
    /* Print infos */
    puts( XSTR(PROGNAME)" "XSTR(VERSION)"-"XSTR(CODENAME) );
    puts("Press Ctrl+C to exit.\n");

    return EXIT_SUCCESS;
}
