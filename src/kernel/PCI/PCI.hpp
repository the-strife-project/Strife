#ifndef PCI_H
#define PCI_H

#include <common/types.hpp>

#define PCI_CONFIG_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC
#define PCI_EMPTY_SLOT 0xFFFF

uint16_t PCI_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void PCI_write(int8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
uint8_t PCI_hasFunctions(uint8_t bus, uint8_t slot);
struct PCI_DeviceDescriptor PCI_getDD(uint8_t bus, uint8_t slot, uint8_t func);
void PCI_loadDrivers();

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
