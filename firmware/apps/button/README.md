# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create button
newt target set button app=apps/button
newt target set button bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build button
newt create-image button 0.0.0.0
newt load button
```