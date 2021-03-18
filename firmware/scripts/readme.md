Install gcc, gdb
```
sudo apt-get install gcc-arm-none-eabi
sudo ln -s /usr/bin/gdb-multiarch /usr/bin/arm-none-eabi-gdb
```

Install J-Link Software and Documentation Pack from SEGGER

Install pylink-square
```
pip install pylink-square
```

```
./load.py rtls anchor
./load.py rtls tag
./load.py gateway gateway
./load.py relay relay
./load.py demo_ccp relay
./load.py demo_tdma relay
./load.py wireshark wireshark
```
