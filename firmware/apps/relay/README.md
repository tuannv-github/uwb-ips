```
newt target create relay
newt target set relay app=apps/relay
newt target set relay bsp=hw/bsp/nordic_pca10040_464
newt target set relay build_profile=optimized
newt build relay
newt create-image relay 0.0.0.0
newt load relay
```