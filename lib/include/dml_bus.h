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
	DMLDevice *dev;
};

class DMLBus {
public:
	DMLBus(void) {portlist.clear();}
	virtual ~DMLBus(void) {portlist.clear();}
	virtual void connectDevice(DMLDevice *dev, uint64_t startAddr, uint32_t len) {
	}
	int readBus(short len, uint32_t addr, void* data) {
		int id = getPortId((uint64_t)addr);
		PortList *port = portlist[id];
		if(port->dev) {
			port->dev->readBankAccess(addr - port->start_address, data, (size_t) len / 8);
		}
		return 0;
	}
	int writeBus(short len, uint32_t addr, uint32_t data) {
		int id = getPortId((uint64_t)addr);
		PortList *port = portlist[id];
		if(port->dev) {
			port->dev->writeBankAccess(addr - port->start_address, &data, (size_t) len / 8);
		}
		return 0;
	}

protected:
	std::vector<PortList*> portlist;

	void addPort(uint64_t startAddr, uint32_t len, DMLDevice *dev = NULL)
	{
		PortList *newport = new PortList;
		newport->start_address = startAddr;
		newport->len = len;
		newport->dev = dev;
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
