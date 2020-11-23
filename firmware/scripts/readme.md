```
conf_template   = "newt target set %s syscfg=RTR_ROLE=RTR_ANCHOR:BLE_MESH=0"
cmd = conf_template % (argv[0])
os.system(cmd)
```
