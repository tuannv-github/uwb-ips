
Erase flash:
```
    $ nrfjprog -f NRF52 -e
```


Build the new bootloader application for the DWM1001 target:

```
newt target create dwm1001_boot
newt target set dwm1001_boot app=@mcuboot/boot/mynewt
newt target set dwm1001_boot bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt target set dwm1001_boot build_profile=optimized
newt build dwm1001_boot
newt load dwm1001_boot
```

```
newt target create nrf52_boot
newt target set nrf52_boot bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt target set nrf52_boot build_profile=optimized
newt target set nrf52_boot app=@mcuboot/boot/mynewt
newt build nrf52_boot
newt load nrf52_boot
```

```
JLinkGDBServer -device NRF52 -if swd -endian little -port 2431 -swoport 2432 -telnetport 2433 -RTTTelnetport 2434 -vd -noir -strict -timeout 0 -select USB=760085865
````