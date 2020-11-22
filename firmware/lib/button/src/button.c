#include <button/button.h>

typedef void (*button_handler_t)(int pin, void *arg);

struct _button_t;

typedef struct _button_arg_t
{
    struct _button_t *button;
    int pin;

    struct hal_timer *timer;
    struct dpl_event *event;
    button_handler_t handler;
    int state;
    void *arg;
}button_arg_t;

typedef struct _button_t{
    struct dpl_eventq eventq;                         
    struct dpl_task task;            
    dpl_stack_t task_stack[MYNEWT_VAL(BUTTON_TASK_STACK_SZ)];
    button_arg_t button_arg[MYNEWT_VAL(BUTTON_NUM)];
}button_t;

static button_t g_button;

void button_irq_handler(void *arg){
    button_arg_t *button_arg = (button_arg_t *)arg;
    button_arg->state = hal_gpio_read(button_arg->pin);
    dpl_cputime_timer_relative(button_arg->timer, MYNEWT_VAL(BUTTON_DEBOUNCE_US));
}

static void
btn_timer_irq(void * arg){
    button_arg_t *button_arg = (button_arg_t *)arg;
    int val = hal_gpio_read(button_arg->pin);
    if(button_arg->state == val){
        dpl_eventq_put(&button_arg->button->eventq, button_arg->event);
    }
}

static void
btn_event_fn(struct dpl_event *ev){
    button_arg_t *button_arg = (button_arg_t*)dpl_event_get_arg(ev);
    button_arg->handler(button_arg->pin, button_arg->arg);
}

int
button_init(int pin, button_handler_t handler, void *arg,
                  hal_gpio_irq_trig_t trig, hal_gpio_pull_t pull){

    assert(handler);

    if(pin >= MYNEWT_VAL(BUTTON_NUM)) return -1;

    g_button.button_arg[pin].button = &g_button;
    g_button.button_arg[pin].pin = pin;

    g_button.button_arg[pin].arg = arg;
    g_button.button_arg[pin].handler = handler;

    g_button.button_arg[pin].timer = calloc(sizeof(struct hal_timer), 1);
    dpl_cputime_timer_init(g_button.button_arg[pin].timer, btn_timer_irq, (void *)&g_button.button_arg[pin]);

    g_button.button_arg[pin].event = calloc(sizeof(struct dpl_event), 1);
    dpl_event_init(g_button.button_arg[pin].event, btn_event_fn, (void *)&g_button.button_arg[pin]);

    hal_gpio_init_in(pin, pull);
    hal_gpio_irq_init(pin, button_irq_handler, &g_button.button_arg[pin], trig, pull);
    hal_gpio_irq_enable(pin);
    return 0;
}

static void *
task(void *arg)
{
    button_t *inst = arg;
    while (1) {
        dpl_eventq_run(&inst->eventq);
    }
    return NULL;
}

void
button_pkg_init(void)
{
#if MYNEWT_VAL(UWB_PKG_INIT_LOG)
    printf("{\"utime\": %"PRIu32",\"msg\": \"button_pkg_init\"}\n",
           (uint32_t)dpl_cputime_ticks_to_usecs(dpl_cputime_get32()));
#endif

    /* Check if the tasks are already initiated */
    if (!dpl_eventq_inited(&g_button.eventq))
    {
        /* Use a dedicate event queue for ccp events */
        dpl_eventq_init(&g_button.eventq);
        dpl_task_init(&g_button.task, "button",
                      task,
                      (void *) &g_button,
                      MYNEWT_VAL(BUTTON_TASK_PRIORITY), 
                      DPL_WAIT_FOREVER,
                      g_button.task_stack,
                      MYNEWT_VAL(BUTTON_TASK_STACK_SZ));
    }
}

int
button_pkg_down(int reason)
{
    if (dpl_eventq_inited(&g_button.eventq))
    {
        dpl_eventq_deinit(&g_button.eventq);
        dpl_task_remove(&g_button.task);
    }

    return 0;
}