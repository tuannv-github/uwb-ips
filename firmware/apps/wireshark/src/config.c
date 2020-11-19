#include <stdio.h>

#include <wireshark/config.h>

static char *lstnr_get(int argc, char **argv, char *val, int val_len_max);
static int lstnr_set(int argc, char **argv, char *val);
static int lstnr_commit(void);
static int lstnr_export(void (*export_func)(char *name, char *val), enum conf_export_tgt tgt);

static on_config_update_t gon_config_update;

static struct {
    uint16_t acc_samples;
    uint16_t verbose;
} local_conf = {0};

static struct{
    char acc_samples[8];
    char verbose[8];
} lstnr_config;

static struct conf_handler lstnr_handler = {
    .ch_name = "lstnr",
    .ch_get = lstnr_get,
    .ch_set = lstnr_set,
    .ch_commit = lstnr_commit,
    .ch_export = lstnr_export,
};

static char *
lstnr_get(int argc, char **argv, char *val, int val_len_max)
{
    if (argc == 1) {
        if (!strcmp(argv[0], "acc_samples"))  return lstnr_config.acc_samples;
        if (!strcmp(argv[0], "verbose"))  return lstnr_config.verbose;
    }
    return NULL;
}

static int
lstnr_set(int argc, char **argv, char *val)
{
    if (argc == 1) {
        if (!strcmp(argv[0], "acc_samples")) {
            return CONF_VALUE_SET(val, CONF_STRING, lstnr_config.acc_samples);
        }
        if (!strcmp(argv[0], "verbose")) {
            return CONF_VALUE_SET(val, CONF_STRING, lstnr_config.verbose);
        }
    }
    return -1;
}

static int
lstnr_commit(void)
{
    conf_value_from_str(lstnr_config.acc_samples, CONF_INT16,
                        (void*)&(local_conf.acc_samples), 0);
    conf_value_from_str(lstnr_config.verbose, CONF_INT16,
                        (void*)&(local_conf.verbose), 0);
    if(gon_config_update != NULL) gon_config_update();
    return 0;
}

static int
lstnr_export(void (*export_func)(char *name, char *val),
             enum conf_export_tgt tgt)
{
    export_func("lstnr/acc_samples", lstnr_config.acc_samples);
    export_func("lstnr/verbose", lstnr_config.verbose);
    return 0;
}

int app_conf_init(on_config_update_t on_config_update){
    int ret;
    ret = conf_register(&lstnr_handler);
    if (ret!=0) return ret;

    ret = conf_load();
    return ret;
}

uint16_t app_conf_get_acc_samples(){
    return local_conf.acc_samples;
}

uint16_t app_conf_get_verbose(){
    return local_conf.verbose;
}