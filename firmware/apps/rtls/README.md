# Real time localization system

## Overview

### Building target

Master node (only one allowed per network):
```no-highlight
newt target create rtls
newt target set rtls app=apps/rtls
newt target set rtls bsp=@decawave-uwb-core/hw/bsp/dwm1001
newt build rtls
newt create-image rtls 0.0.0.0
newt load rtls
```