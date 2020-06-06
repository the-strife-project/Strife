#ifndef PCI_H
#define PCI_H

#include <common/types.hpp>
#include <kernel/klibc/STL/list>

#define PCI_CONFIG_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC
#define PCI_EMPTY_SLOT 0xFFFF

class PCI {
public:
	// Thanks to: https://wiki.osdev.org/PCI
	struct Descriptor0 {
		uint16_t vendorID;
		uint16_t deviceID;
		uint16_t command;
		uint16_t status;
		uint8_t revisionID;
		uint8_t progIF;
		uint8_t subclass;
		uint8_t classCode;
		uint8_t cacheLineSize;
		uint8_t latencyTimer;
		uint8_t headerType;	// Must be zero if we're at this point.
		uint8_t BIST;
		uint32_t BAR[6];
		uint32_t cardbusCISptr;
		uint16_t subsystemVendorID;
		uint16_t subsystemID;
		uint32_t expansionROMbaseaddr;
		uint8_t capabilitiesptr;
		uint8_t reserved[7];
		uint8_t interruptLine;
		uint8_t interruptPin;
		uint8_t minGrant;
		uint8_t maxLatency;
	};

private:
	uint8_t bus, slot, func;
	Descriptor0 desc0;
	void prepare(uint8_t offset);
	uint16_t read(uint8_t offset);

public:
	PCI();
	PCI(uint8_t bus, uint8_t slot, uint8_t func);

	uint8_t getBus();
	uint8_t getSlot();
	uint8_t getFunc();
	Descriptor0 getDescriptor();
	bool exists();
};

list<PCI> PCI_probe();

struct PCI_DeviceDescriptor {
	uint32_t portBase;
	uint32_t interrupt;

	uint8_t bus;
	uint8_t slot;
	uint8_t function;

	uint16_t vendor_id;
	uint16_t device_id;

	uint8_t class_id;
	uint8_t subclass_id;
	uint8_t interface_id;

	uint8_t revision;
};

#endif
