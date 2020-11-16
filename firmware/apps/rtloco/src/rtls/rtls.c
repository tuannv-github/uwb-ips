#include <rtloco/rtls.h>

#if __GNUC__ >= 9
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
#include <message/mavlink/protocol/mavlink.h>

#include <uwb/uwb.h>
#include <uwb_ccp/uwb_ccp.h>

float g_location[3] = {0.2, 123.456, 567};
uint16_t g_address = 0x1234;
uint16_t g_ntype = ANCHOR;
struct uwb_dev *udev;

void rtls_get_location(float *x, float *y, float *z){
    *x = g_location[0];
    *y = g_location[1];
    *z = g_location[2];
}

void rtls_set_location(float x, float y, float z){
    g_location[0] = x;
    g_location[1] = y;
    g_location[2] = z;
}

void rtls_get_address(uint16_t *address){
    *address = udev->my_short_address;
}

void rtls_get_ntype(uint8_t *ntype){
    *ntype = g_ntype;
}

static bool
superframe_cb(struct uwb_dev * inst, struct uwb_mac_interface * cbs)
{
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance *)cbs->inst_ptr;
    printf("{\"role: %d, utime\": %ld,\"msg\": \"ccp:superframe_cb\", \"period\": %ld}\n",
            ccp->config.role, dpl_cputime_ticks_to_usecs(dpl_cputime_get32()), (uint32_t)uwb_dwt_usecs_to_usecs(ccp->period));

    return false;
}

static struct uwb_mac_interface cbs;
void rtls_init(){
    udev = uwb_dev_idx_lookup(0);
    struct uwb_ccp_instance *ccp = (struct uwb_ccp_instance*)uwb_mac_find_cb_inst_ptr(udev, UWBEXT_CCP);
    rtls_ccp_start(ccp);

    uwb_set_uid(udev, udev->my_short_address);

    cbs = (struct uwb_mac_interface){
        .id =  UWBEXT_APP0,
        .inst_ptr = ccp,
        .superframe_cb = superframe_cb,
    };
    uwb_mac_append_interface(udev, &cbs);
}