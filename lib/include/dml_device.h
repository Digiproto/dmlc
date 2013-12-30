/*
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    dml_device.h
 * Email:   lyjforjob@gmail.com
 * Date:    2013-12-22
 */
#ifndef __DML_DEVICE_H__
#define __DML_DEVICE_H__

typedef struct bank_access bank_access_t;

class DMLDevice {
public:
	DMLDevice(const char *objname, const char *classname);
	//~DMLDevice();
	conf_object_t* getObj(void);
	int readBankAccess(uint32_t addr, void *buf, size_t size);
	int writeBankAccess(uint32_t addr, const void *buf, size_t size);
protected:
	conf_object_t       *obj;
	const bank_access_t *iface;
	void addBankAccess(void);
};

#endif
