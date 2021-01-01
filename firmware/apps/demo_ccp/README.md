```no-highlight
newt target create demo_ccp
newt target set demo_ccp app=apps/demo_ccp
newt target set demo_ccp bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build demo_ccp
newt create-image demo_ccp 0.0.0.0
newt load demo_ccp
```