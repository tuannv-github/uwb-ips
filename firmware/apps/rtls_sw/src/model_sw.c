#include <assert.h>
#include "os/mynewt.h"
#include "mesh/mesh.h"
#include "console/console.h"
#include "hal/hal_system.h"
#include "hal/hal_gpio.h"
#include "bsp/bsp.h"
#include "shell/shell.h"

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/glue.h"

#include <rtls_sw/model.h>
#include <rtls_sw/ble_mesh.h>

static
int loc_data_update(struct bt_mesh_model *mod){
    console_printf("#mesh loc_data_update\n");
    return 0;
}
static struct bt_mesh_model_pub gen_onoff_pub = {
    .period = BT_MESH_PUB_PERIOD_SEC(1),
    .update = loc_data_update,
    .ttl = BT_MESH_TTL_DEFAULT,
    .msg = (struct os_mbuf *)100,
};

static void gen_onoff_get(struct bt_mesh_model *model,
              struct bt_mesh_msg_ctx *ctx,
              struct os_mbuf *buf)
{
    console_printf("#mesh-level STATUS\n");
}


static const struct bt_mesh_model_op gen_onoff_op[] = {
    { BT_MESH_MODEL_OP_2(0x82, 0x04), 0, gen_onoff_get },
    BT_MESH_MODEL_OP_END,
};

struct bt_mesh_model model_sw[1] = {
    BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_CLI, gen_onoff_op, &gen_onoff_pub, NULL),
};

void handler(void *arg){
    console_printf("BTN 1 click\n");
}

void mode_sw_init(){
    hal_gpio_init_in(BUTTON_1, HAL_GPIO_PULL_NONE);
    hal_gpio_irq_init(BUTTON_1, handler, NULL, HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_NONE);
    hal_gpio_irq_enable(BUTTON_1);
}