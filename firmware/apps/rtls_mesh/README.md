# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls_mesh
newt target set rtls_mesh app=apps/rtls_mesh
newt target set rtls_mesh bsp=@decawave-uwb-core/hw/bsp/dwm1001

newt target set rtls_mesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'
newt build rtls_mesh
newt create-image rtls_mesh 0.0.0.0
newt load rtls_mesh

newt target set rtls_mesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x22, 0}'
newt build rtls_mesh
newt create-image rtls_mesh 0.0.0.0
newt load rtls_mesh
```


```no-highlight
newt target create rtls_mesh
newt target set rtls_mesh app=apps/rtls_mesh
newt target set rtls_mesh bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend rtls_mesh syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build rtls_mesh
newt create-image rtls_mesh 0.0.0.0
newt load rtls_mesh
```

```
newt target create blemesh
newt target set blemesh app=@apache-mynewt-nimble/apps/blemesh
newt target set blemesh bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target set blemesh build_profile=optimized

newt target set blemesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set blemesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build blemesh
newt create-image blemesh 0.0.0.0
newt load blemesh
```

```
newt target create blemesh
newt target set blemesh app=@apache-mynewt-nimble/apps/blemesh
newt target set blemesh bsp=hw/bsp/dwm1001_232
newt target set blemesh build_profile=optimized

newt target set blemesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set blemesh syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build blemesh
newt create-image blemesh 0.0.0.0
newt load blemesh
```

```
newt load blemesh --extrajtagcmd "-select USB=760085863" 
```