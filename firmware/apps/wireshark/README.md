# UWB sniffer / listener

## Overview
This example is really a very useful tool for assisting you develop UWB applications. It allows you to monitor the traffic and get accurate
timestamps for each package. 

### Building target for dwm1001

```no-highlight
newt target create wireshark
newt target set wireshark app=apps/wireshark
newt target set wireshark bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build wireshark
newt create-image wireshark 0.0.0.0
newt load wireshark
```
