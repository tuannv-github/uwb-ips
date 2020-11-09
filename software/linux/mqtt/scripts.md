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