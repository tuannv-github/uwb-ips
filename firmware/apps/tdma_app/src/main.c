#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <uwb_ccp/uwb_ccp.h>
#include <tdma/tdma.h>

int main(int argc, char **argv){
    int rc;
    
    sysinit();
    conf_load();

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);

    tdma_instance_t *tdma = (tdma_instance_t*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_TDMA);
    assert(tdma);
    rtls_ccp_start(tdma->ccp);

    struct uwb_mac_interface cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = tdma,
    };
    uwb_mac_append_interface(udev, &cbs);
    
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}