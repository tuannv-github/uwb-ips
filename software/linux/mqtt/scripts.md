```
python3 -m venv env
virtualenv --system-site-packages
```

```
source env/bin/activate
```

```
export PYTHONPATH=
pip freeze > requirements.txt
```

```
sudo apt-get install mosquitto
sudo apt-get install mosquitto-clients

mosquitto_sub -t "test"
mosquitto_pub -m "message from mosquitto_pub client" -t "test"
```

```
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

# Install mosquitto with websockets
```
sudo apt-get update
sudo apt-get install build-essential python quilt python-setuptools python3
sudo apt-get install libssl-dev
sudo apt-get install cmake
sudo apt-get install libc-ares-dev
sudo apt-get install uuid-dev
sudo apt-get install daemon
sudo apt-get install libwebsockets-dev
```
```
cd Downloads/
wget http://mosquitto.org/files/source/mosquitto-1.6.9.tar.gz
tar zxvf mosquitto-1.6.9.tar.gz
cd mosquitto-1.6.9
```
```
make
sudo make install
sudo cp mosquitto.conf /etc/mosquitto
```

/etc/mosquitto/mosquitto.conf
```
listener 1883
protocol mqtt

listener 8080
protocol websockets
```

```
/usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
```

```
socat -d -d pty,raw,echo=0 pty,raw,echo=0
sudo socat -u -u pty,raw,echo=0,link=/dev/ttyS20 pty,raw,echo=0,link=/dev/ttyS21
sudo chown tuannv:tuannv /dev/ttyS20 /dev/ttyS21
```

Protocol generator
```
cd mavlink-1.0.13
python -m pymavlink.tools.mavgen --lang=Python --wire-protocol=1.0 --output=../mavlink.py ../protocol.xml
python -m pymavlink.tools.mavgen --lang=C --wire-protocol=1.0 --output=../mavlink/ ../protocol.xml
```