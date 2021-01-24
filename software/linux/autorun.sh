#!/bin/bash
python3 /home/debian/indoor-positioning-system/software/linux/gateway/gateway.py > /home/debian/indoor-positioning-system/software/linux/gateway/gateway.log 2>&1 &
python3 /home/debian/indoor-positioning-system/software/linux/web-server/manage.py runserver 0.0.0.0:80 > /home/debian/indoor-positioning-system/software/linux/web-server/server.log 2>&1
