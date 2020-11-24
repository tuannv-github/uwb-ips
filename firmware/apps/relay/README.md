## Building target for dwm1001

```no-highlight
newt target create relay
newt target set relay app=apps/relay
newt target set relay bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build relay
newt create-image relay 0.0.0.0
newt load relay
```


```no-highlight
newt target create relay
newt target set relay app=apps/relay
newt target set relay bsp=@apache-mynewt-core/hw/bsp/nordic_pca10040
newt build relay
newt create-image relay 0.0.0.0
newt load relay
```