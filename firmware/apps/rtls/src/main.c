#include <os/mynewt.h>
#include <config/config.h>

#include <uwb/uwb.h>
#include <uwb_ccp/uwb_ccp.h>
#include <dpl/dpl.h>

static struct dpl_event nrng_complete_event;

static void 
nrng_complete_cb(struct dpl_event *ev) {
    assert(ev != NULL);
    assert(dpl_event_get_arg(ev) != NULL);
}

static bool 
complete_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    if(inst->fctrl != FCNTL_IEEE_RANGE_16){
        return false;
    }
    dpl_eventq_put(dpl_eventq_dflt_get(), &nrng_complete_event);
    return true;
}

int main(int argc, char **argv){
    int rc;

    sysinit();
    conf_load();

    /* Get ultra wide band device instance */
    struct uwb_dev *udev = uwb_dev_idx_lookup(0);
    assert(udev);

    /* Disable auto RX and double buffer */
    udev->config.rxauto_enable = false;
    udev->config.dblbuffon_enabled = false;
    uwb_set_dblrxbuff(udev, udev->config.dblbuffon_enabled);
    
    /* Get clock calibration instance */
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_CCP);
    assert(ccp);

    /* Get personal area network instance */
    struct uwb_pan_instance *pan = (struct uwb_pan_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_PAN);
    assert(pan);

    /* Get personal ranging instance */
    struct uwb_rng_instance* rng = (struct uwb_rng_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_RNG);
    assert(rng);

    /* Get network ranging instance */
    struct nrng_instance* nrng = (struct nrng_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_NRNG);
    assert(nrng);

    /* Set callback for ranging request complete event */
    dpl_event_init(&nrng_complete_event, nrng_complete_cb, nrng);
    struct uwb_mac_interface cbs = (struct uwb_mac_interface){
        .id = UWBEXT_APP0,
        .inst_ptr = nrng,
        .complete_cb = complete_cb
    };
    uwb_mac_append_interface(udev, &cbs);

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}