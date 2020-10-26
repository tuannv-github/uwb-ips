# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create tdma_app
newt target set tdma_app app=apps/tdma_app
newt target set tdma_app bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build tdma_app
newt create-image tdma_app 0.0.0.0
newt load tdma_app
```