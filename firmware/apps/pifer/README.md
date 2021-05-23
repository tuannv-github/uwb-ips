# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create pifer
newt target set pifer app=apps/pifer
newt target set pifer bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build pifer
newt create-image pifer 0.0.0.0
newt load pifer
```
