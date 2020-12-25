#include <serial/serial.h>
#include <uart/uart.h>
#include <rbuf/rbuf.h>

typedef struct _serial_t{
    struct uart_conf uart_cfg;
    struct uart_dev *uart_dev;
    rbuf_t rbuf_tx;
    uint8_t buffer_tx[MYNEWT_VAL(SERIAL_TX_RBUF_SIZE)];
    rbuf_t rbuf_rx;
    uint8_t buffer_rx[MYNEWT_VAL(SERIAL_RX_RBUF_SIZE)];

    struct dpl_sem sem_rx;
}serial_t;

static serial_t g_serial;

static int
serial_tx_char(void *arg)
{
    serial_t *serial = (serial_t *)arg;
    char chr;
    if(!rbuf_get(&serial->rbuf_tx, &chr)){
        return chr;
    }
    return -1;
}

static void
serial_tx_done(void *arg)
{
    serial_t *serial = (serial_t *)arg;
    if(!rbuf_empty(&serial->rbuf_tx)){
        uart_start_tx(serial->uart_dev);
    }
}

static int
serial_rx_char(void *arg, uint8_t byte)
{
    serial_t *serial = (serial_t *)arg;
    if(rbuf_put(&serial->rbuf_rx, byte)){
        printf("RX buffer full\n");
    }
    dpl_sem_release(&serial->sem_rx);
    return 0;
}

char serial_read(){
    dpl_sem_pend(&g_serial.sem_rx, DPL_TIMEOUT_NEVER);
    char chr;
    rbuf_get(&g_serial.rbuf_rx, &chr);
    return chr;
}

void serial_read_line(char *line, size_t len){
    char chr;
    size_t idx = 0;
    while(1){
        chr = serial_read();
        if(idx==0 && (chr=='\n' || chr=='\r')) continue;
        if(chr != '\r' && chr != '\n' && idx < len){
            line[idx++] = chr;
        }
        else{
            line[idx] = 0;
            break;
        }
    }
}

void 
serial_write(char *chr, size_t len){
    size_t idx;
    for(idx=0; idx<len; idx++){
        if(rbuf_put(&g_serial.rbuf_tx, chr[idx])){
            printf("TX buffer full\n");
        }
    }
    uart_start_tx(g_serial.uart_dev);
}

void 
serial_write_str(char *chr){
    while(*chr!=0){
        rbuf_put(&g_serial.rbuf_tx, *chr++);
    }
    uart_start_tx(g_serial.uart_dev);
}

void 
serial_write_line(char *line){
    while(*line!=0){
        rbuf_put(&g_serial.rbuf_tx, *line++);
    }
    rbuf_put(&g_serial.rbuf_tx, '\r');
    rbuf_put(&g_serial.rbuf_tx, '\n');
    uart_start_tx(g_serial.uart_dev);
}

void
serial_pkg_init(void)
{
    #if MYNEWT_VAL(UWB_PKG_INIT_LOG)
    printf("{\"utime\": %lu,\"msg\": \"button_pkg_init\"}\n",
           dpl_cputime_ticks_to_usecs(dpl_cputime_get32()));
    #endif

    rbuf_init(&g_serial.rbuf_tx, g_serial.buffer_tx, MYNEWT_VAL(SERIAL_TX_RBUF_SIZE));
    rbuf_init(&g_serial.rbuf_rx, g_serial.buffer_rx, MYNEWT_VAL(SERIAL_RX_RBUF_SIZE));

    g_serial.uart_cfg = (struct uart_conf){
        .uc_speed = MYNEWT_VAL(SERIAL_UART_BAUD),
        .uc_databits = 8,
        .uc_stopbits = 1,
        .uc_parity = UART_PARITY_NONE,
        .uc_flow_ctl = UART_FLOW_CTL_NONE,
        .uc_tx_char = serial_tx_char,
        .uc_rx_char = serial_rx_char,
        .uc_tx_done = serial_tx_done,
        .uc_cb_arg = (void*)(&g_serial)
    };

    g_serial.uart_dev = (struct uart_dev *)os_dev_open(MYNEWT_VAL(SERIAL_UART_DEV), OS_TIMEOUT_NEVER, &g_serial.uart_cfg);

    if (!g_serial.uart_dev) {
        printf("Serial device %s: not found", MYNEWT_VAL(SERIAL_UART_DEV));
        return;
    }

    dpl_sem_init(&g_serial.sem_rx, 0x00);
}

int
serial_pkg_down(int reason)
{
    return 0;
}