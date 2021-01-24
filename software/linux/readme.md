# Auto run on startup

Open service file:
```
vi /etc/systemd/system/rtls.service
```

Content:
```
[Unit]
Description=RTLS script
After=network.target

[Service]
Type=oneshot
ExecStart=/home/debian/indoor-positioning-system/software/linux/autorun.sh

[Install]
WantedBy=multi-user.target
```

Run the following commands:
```
sudo systemctl daemon-reload
sudo systemctl enable rtls
sudo systemctl start rtls
sudo systemctl status rtls
```