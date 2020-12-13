#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <rtls_ccp/rtls_ccp.h>

static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    // printf("\33[2K\r{\"role: %d, utime\": %ld,\"msg\": \"ccp:superframe_cb\", \"period\": %ld}",
    //         ccp->config.role, dpl_cputime_ticks_to_usecs(dpl_cputime_get32()), (uint32_t)uwb_dwt_usecs_to_usecs(ccp->period));
    printf("{\"role: %d, utime\": %ld,\"msg\": \"ccp:superframe_cb\", \"period\": %ld}\n",
            ccp->config.role, dpl_cputime_ticks_to_usecs(dpl_cputime_get32()), (uint32_t)uwb_dwt_usecs_to_usecs(ccp->period));

    return false;
}

static void 
uwb_ccp_sync_cb(ccp_sync_t ccp_sync, void *arg){
    switch (ccp_sync)
    {
    case CCP_SYNC_SYED:
        printf("CCP_SYNC_SYED\n");
        break;
    case CCP_SYNC_LOST:
        printf("CCP_SYNC_LOST\n");
        break;
    }
}

int main(int argc, char **argv){

    sysinit();
    conf_load();

    printf(" \n");
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);

    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_CCP);
    assert(ccp);

    rtls_ccp_start_role(ccp, CCP_ROLE_MASTER);
    rtls_ccp_set_sync_cb(ccp, uwb_ccp_sync_cb, NULL);

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
}