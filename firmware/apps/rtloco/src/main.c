#include <os/mynewt.h>
#include <config/config.h>

#include <rtloco/mesh_if.h>
#include <rtloco/rtls.h>

#include <uwb/uwb.h>
#include <uwb_ccp/uwb_ccp.h>

int main(int argc, char **argv){
    int rc;
    
    sysinit();
    conf_load();

    rtls_init();
    ble_mesh_init();

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}