from django.shortcuts import render

# Create your views here.
def index(request):
    cxt = {"mqtt_server_addr": "127.0.0.1", "mqtt_server_port": "8080", "location_evt_topic": "location_evt"}
    return render(request, "home/index.html", context=cxt)