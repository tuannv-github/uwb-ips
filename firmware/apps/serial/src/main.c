#include <os/mynewt.h>
#include <serial/serial.h>

#define STACK_SIZE  128
#define PRIORITY    101

struct os_task task;           
os_stack_t task_stack[STACK_SIZE];

static void
task_func(void *arg)
{
    char chr;
    while(1){
        chr = serial_read();
        serial_write(&chr, 1);
    }
}

int main(int argc, char **argv){
    sysinit();

    printf("App: gateway\n");

    os_task_init(&task, "echoback",
                task_func,
                NULL,
                PRIORITY, 
                OS_WAIT_FOREVER,
                task_stack,
                STACK_SIZE);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
}