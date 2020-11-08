#include <dpl/dpl.h>
#include <os/mynewt.h>
#include <bsp/bsp.h>

#include <button/button.h>

void button_handler(int pin, void *arg){
    printf("Button: %d\n", pin);
}

int main(int argc, char **argv){
    sysinit();

    button_init(BUTTON_2, button_handler, NULL, HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_UP);

    while (1) {
        dpl_eventq_run(dpl_eventq_dflt_get());
    }
}