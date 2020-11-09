# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls_node_anchor
newt target set rtls_node_anchor app=apps/rtls_node_anchor
newt target set rtls_node_anchor bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtls_node_anchor
newt create-image rtls_node_anchor 0.0.0.0
newt load rtls_node_anchor
```