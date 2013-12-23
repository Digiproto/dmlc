/*
 * © Copyright 2013 eJim Lee. All Rights Reserved.
 * File:    dml_bus.h
 * Email:   lyjforjob@gmail.com
 * Date:    2013-12-22
 */
#ifndef __DML_BUS_H__
#define __DML_BUS_H__

#include <vector>
#include "dml_device.h"

typedef struct PortList PortList;
struct PortList {
	uint64_t start_address;
	uint32_t len;
};

class DMLBus {
public:
	DMLBus(void) {portlist.clear();}
	//~DMLBus(void) {portlist.clear();}
	virtual void connectDevice(DMLDevice *dev, uint64_t startAddr, uint32_t len) {
	}

protected:
	std::vector<PortList*> portlist;

	void addPort(uint64_t startAddr, uint32_t len)
	{
		PortList *newport = new PortList;
		newport->start_address = startAddr;
		newport->len = len;
		portlist.push_back(newport);
	}
	int getPortId(uint64_t address)
	{
		int len = portlist.size();
		int i;
		for(i = 0; i < len; i++) {
			PortList *port = portlist[i];
			if(port->start_address <= address &&
					address < port->start_address + port->len) {
				return i;
			}
		}
		return -1;
	}
};

#endif
