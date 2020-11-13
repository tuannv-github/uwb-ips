# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create hello
newt target set hello app=apps/hello
newt target set hello bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build hello
newt create-image hello 0.0.0.0
newt load hello
```

```no-highlight
newt target create hello
newt target set hello app=apps/hello
newt target set hello bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend hello syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build hello
newt create-image hello 0.0.0.0
newt load hello
```