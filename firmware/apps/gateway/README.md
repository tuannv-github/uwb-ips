```
newt target create gateway
newt target set gateway app=apps/gateway
newt target set gateway bsp=hw/bsp/nordic_pca10040_464
newt build gateway
newt create-image gateway 0.0.0.0
newt load gateway
```
