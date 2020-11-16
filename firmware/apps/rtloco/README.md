# UWB clock calibration packet master example

## Overview
rtloco, ccp_relay, ccp_slave examples illustrate simple usage of ccp.

## Building target for dwm1001

```no-highlight
newt target create rtloco
newt target set rtloco app=apps/rtloco
newt target set rtloco bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtloco
newt create-image rtloco 0.0.0.0
newt load rtloco
```

```
newt target create rtloco
newt target set rtloco app=apps/rtloco
newt target set rtloco bsp=hw/bsp/dwm1001_464
newt build rtloco
newt create-image rtloco 0.0.0.0
newt load rtloco
```