# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtls
newt target set rtls app=apps/rtls
newt target set rtls bsp=@decawave-uwb-core/hw/bsp/dwm1001

newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'
newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls

newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x22, 0}'
newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```


```no-highlight
newt target create rtls
newt target set rtls app=apps/rtls
newt target set rtls bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend rtls syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```

```
newt target create rtls
newt target set rtls app=@apache-mynewt-nimble/apps/rtls
newt target set rtls bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target set rtls build_profile=optimized

newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```

```
newt target create rtls
newt target set rtls app=@apache-mynewt-nimble/apps/rtls
newt target set rtls bsp=hw/bsp/dwm1001_232
newt target set rtls build_profile=optimized

newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set rtls syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```

```
newt load rtls --extrajtagcmd "-select USB=760085863" 
```

```
newt target create rtls
newt target set rtls app=apps/rtls
newt target set rtls bsp=hw/bsp/dwm1001_464
newt target set rtls build_profile=optimized
newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```

```
newt target set rtls syscfg=BLE_MESH=0
```