Link to extcap folder from firmware folder:
```
sudo ln -s  `pwd`/apps/wireshark/script/uwb_sniffer.py /usr/lib/x86_64-linux-gnu/wireshark/extcap/uwb_sniffer
```

```
./uwb_sniffer --extcap-interfaces
```
