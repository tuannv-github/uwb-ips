```no-highlight
newt target create wireshark
newt target set wireshark app=apps/wireshark
newt target set wireshark bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build wireshark
newt create-image wireshark 0.0.0.0
newt load wireshark
```
