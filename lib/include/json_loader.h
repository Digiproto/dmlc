#ifndef __JSON_SYSTEMC_H__
#define __JSON_SYSTEMC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dml_bus.h"
#include "dml_device.h"

/* need implement */
typedef DMLDevice* (*NewDevice)(const char *objname, const char *classname);
/*
 * if we have:
 * class SimicsDevice : public DMLDevice;
 *
 * implement:
 * DMLDevice* create_device(const char *objname, const char *classname)
 * {
 * 		return new SimicsDevice(objname, classname);
 * }
 */

typedef void (*interrupt_fn)(conf_object_t *obj, int irq);
typedef void (*interrupt_clear_fn)(conf_object_t *obj, int irq);

typedef struct InterruptIntf {
	interrupt_fn       intr;
	interrupt_clear_fn intr_clear;
}InterruptIntf;

typedef struct ExtraSystemCData {
	NewDevice     create_fn;
	DMLBus       *bus;
	InterruptIntf intr;
}ExtraSystemCData;

#ifdef __cplusplus
}
#endif

#endif
