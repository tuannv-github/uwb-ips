from django.shortcuts import render
import socket

# Create your views here.
def index(request):
    cxt = {"mqtt_server_addr": request.get_host(), "mqtt_server_port": "8080", 
            "topic_subscribe": "topic_evt", 
            "topic_publish": "topic_cmd"}
    return render(request, "home/index.html", context=cxt)