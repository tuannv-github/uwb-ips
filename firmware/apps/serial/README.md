# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create serial
newt target set serial app=apps/serial
newt target set serial bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build serial
newt create-image serial 0.0.0.0
newt load serial
```

```no-highlight
newt target create serial
newt target set serial app=apps/serial
newt target set serial bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt build serial
newt create-image serial 0.0.0.0
newt load serial
```