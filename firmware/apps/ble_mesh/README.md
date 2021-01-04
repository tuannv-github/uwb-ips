```no-highlight
newt target create ble_mesh
newt target set ble_mesh app=apps/ble_mesh
newt target set ble_mesh bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build ble_mesh
newt create-image ble_mesh 0.0.0.0
newt load ble_mesh
```