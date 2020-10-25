# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create ccp_master
newt target set ccp_master app=apps/ccp_master
newt target set ccp_master bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build ccp_master
newt create-image ccp_master 0.0.0.0
newt load ccp_master
```