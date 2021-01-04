# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create am2320
newt target set am2320 app=apps/am2320
newt target set am2320 bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build am2320
newt create-image am2320 0.0.0.0
newt load am2320
```

```no-highlight
newt target create am2320
newt target set am2320 app=apps/am2320
newt target set am2320 bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend am2320 syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build am2320
newt create-image am2320 0.0.0.0
newt load am2320
```