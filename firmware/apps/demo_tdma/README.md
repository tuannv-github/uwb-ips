# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create demo_tdma
newt target set demo_tdma app=apps/demo_tdma
newt target set demo_tdma bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build demo_tdma
newt create-image demo_tdma 0.0.0.0
newt load demo_tdma
```