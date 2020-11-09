# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls_node_tag
newt target set rtls_node_tag app=apps/rtls_node_tag
newt target set rtls_node_tag bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtls_node_tag
newt create-image rtls_node_tag 0.0.0.0
newt load rtls_node_tag
```