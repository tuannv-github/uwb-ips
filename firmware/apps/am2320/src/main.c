#include <os/mynewt.h>

int main(int argc, char **argv){
    sysinit();

    printf("Hello world!\n");

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}