# UWB clock calibration packet master example

## Overview
ccp_master, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create gateway
newt target set gateway app=apps/gateway
newt target set gateway bsp=@decawave-uwb-core/hw/bsp/dwm1001

newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'
newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway

newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x22, 0}'
newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```


```no-highlight
newt target create gateway
newt target set gateway app=apps/gateway
newt target set gateway bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target amend gateway syscfg=BLE_MESH_SETTINGS=0:CONFIG_NFF=0
newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```

```
newt target create gateway
newt target set gateway app=@apache-mynewt-nimble/apps/gateway
newt target set gateway bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target set gateway build_profile=optimized

newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```

```
newt target create gateway
newt target set gateway app=@apache-mynewt-nimble/apps/gateway
newt target set gateway bsp=hw/bsp/dwm1001_232
newt target set gateway build_profile=optimized

newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x20, 0}'
newt target set gateway syscfg=BLE_MESH_PB_GATT=1:BLE_MESH_DEV_UUID='(uint8_t[16]){0x22, 0x21, 0}'

newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```

```
newt load gateway --extrajtagcmd "-select USB=760085863" 
```

```
newt target create gateway
newt target set gateway app=apps/gateway
newt target set gateway bsp=hw/bsp/dwm1001_464
newt target set gateway build_profile=optimized
newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```