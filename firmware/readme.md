
Erase flash:
```
    $ nrfjprog -f NRF52 -e
```


Build the new bootloader application for the DWM1001 target:

```no-highlight

newt target create dwm1001_boot
newt target set dwm1001_boot app=@mcuboot/boot/mynewt
newt target set dwm1001_boot bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt target set dwm1001_boot build_profile=optimized
newt build dwm1001_boot
newt load dwm1001_boot

```