# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create mesh_provisioner
newt target set mesh_provisioner app=apps/mesh_provisioner
newt target set mesh_provisioner bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build mesh_provisioner
newt create-image mesh_provisioner 0.0.0.0
newt load mesh_provisioner
```

```no-highlight
newt target create mesh_provisioner
newt target set mesh_provisioner app=apps/mesh_provisioner
newt target set mesh_provisioner bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt build mesh_provisioner
newt create-image mesh_provisioner 0.0.0.0
newt load mesh_provisioner
```