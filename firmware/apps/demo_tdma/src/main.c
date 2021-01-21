#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <rtls_tdma/rtls_tdma.h>

rtls_tdma_instance_t rtls_tdma_instance = {
    .role = RTR_ANCHOR
};

int main(int argc, char **argv){
    int rc;
    
    sysinit();
    conf_load();

    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    printf("{\"utime\": %lu,\"exec\": \"%s\"}\n",utime,__FILE__);
    printf("{\"device_id\"=\"%lX\"",udev->device_id);
    printf(",\"panid=\"%X\"",udev->pan_id);
    printf(",\"addr\"=\"%X\"",udev->uid);
    printf(",\"part_id\"=\"%lX\"",(uint32_t)(udev->euid&0xffffffff));
    printf(",\"lot_id\"=\"%lX\"}\n",(uint32_t)(udev->euid>>32));
    printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime, uwb_phy_SHR_duration(udev));

    rtls_tdma_start(&rtls_tdma_instance, udev);
    
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}