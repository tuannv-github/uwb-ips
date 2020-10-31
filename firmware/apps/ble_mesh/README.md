# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create ble_mesh
newt target set ble_mesh app=apps/ble_mesh
newt target set ble_mesh bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build ble_mesh
newt create-image ble_mesh 0.0.0.0
newt load ble_mesh
```