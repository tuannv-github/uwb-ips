# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls_sw
newt target set rtls_sw app=apps/rtls_sw
newt target set rtls_sw bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtls_sw
newt create-image rtls_sw 0.0.0.0
newt load rtls_sw
```


```no-highlight
newt target create rtls_sw
newt target set rtls_sw app=apps/rtls_sw
newt target set rtls_sw bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend rtls_sw syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build rtls_sw
newt create-image rtls_sw 0.0.0.0
newt load rtls_sw
```