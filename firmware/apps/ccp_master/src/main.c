#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <uwb_ccp/uwb_ccp.h>


/**
 * @fn superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
 * @brief
 *
 * @param inst  Pointer to struct uwb_dev.
 * @param cbs   Pointer to struct uwb_mac_interface.
 *
 * @return bool based on the totality of the handling which is false this implementation.
 */
static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    printf("{\"role: %d, utime\": %"PRIu32",\"msg\": \"ccp:superframe_cb\", \"period\": %"PRIu32"}\n",
            ccp->config.role, dpl_cputime_ticks_to_usecs(dpl_cputime_get32()), (uint32_t)uwb_dwt_usecs_to_usecs(ccp->period));
    
    return false;
}

int main(int argc, char **argv){
    int rc;
    
    sysinit();
    conf_load();

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);

    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_CCP);
    assert(ccp);

    rtls_ccp_start(ccp);

    udev->my_short_address = MYNEWT_VAL(NODE_ADDRESS);
    uwb_set_uid(udev, udev->my_short_address);

    struct uwb_mac_interface cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = ccp,
        .superframe_cb = superframe_cb
    };
    uwb_mac_append_interface(udev, &cbs);
    
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}