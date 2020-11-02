# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls_lb
newt target set rtls_lb app=apps/rtls_lb
newt target set rtls_lb bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtls_lb
newt create-image rtls_lb 0.0.0.0
newt load rtls_lb
```