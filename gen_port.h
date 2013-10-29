#ifndef __GEN_PORT__
#define __GEN_PORT__
#include "gen_common.h"
#include "gen_implement.h"
void gen_device_port_description(device_t *dev, FILE *f);
void gen_device_port_code(device_t *dev, FILE *f);
void gen_device_port_header(device_t *dev, FILE *f);
#endif

