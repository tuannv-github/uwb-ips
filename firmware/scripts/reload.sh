#!/usr/bin/env bash

set -e
echo $(pwd)/env/bin/activate
source $(pwd)/env/bin/activate

$(pwd)/env/bin/python load.py $1
$(pwd)/env/bin/python rtt.py

deactivate