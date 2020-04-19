#include <kernel/PCI/PCI.hpp>
#include <kernel/asm.hpp>
#include <kernel/klibc/stdio>

/*
	31			30-24		23-16		15-11		10-8		7-0
	Enable		Reserved	BUS			Device		Function	Register offset
*/
uint32_t __PCI_getAddress(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	// Compute the address according to the structure above.
	return (uint32_t)(
		(bus << 16) |
		(slot << 11) |
		(func << 8) |
		(offset & 0xFC) |
		((uint32_t)0x80000000)
	);
}

inline void __PCI_prepare(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	outl(PCI_CONFIG_PORT, __PCI_getAddress(bus, slot, func, offset));
}

uint16_t PCI_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	__PCI_prepare(bus, slot, func, offset);

	// Read the data: the first word of the 32 bits register.
	return (uint16_t)(inl(PCI_DATA_PORT) >> ((offset & 2) * 8));
}

void PCI_write(int8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
	__PCI_prepare(bus, slot, func, offset);

	// Write the data.
	outl(PCI_DATA_PORT, value);
}

uint8_t PCI_hasFunctions(uint8_t bus, uint8_t slot) {
	return PCI_read(bus, slot, 0, 0x0E) & (1<<7);
}

struct PCI_DeviceDescriptor PCI_getDD(uint8_t bus, uint8_t slot, uint8_t func) {
	struct PCI_DeviceDescriptor ret;
	ret.bus = bus; ret.slot = slot; ret.function = func;

	ret.vendor_id = PCI_read(bus, slot, func, 0x00);
	ret.device_id = PCI_read(bus, slot, func, 0x02);

	ret.class_id = PCI_read(bus, slot, func, 0x0B);
	ret.subclass_id = PCI_read(bus, slot, func, 0x0A);
	ret.interface_id = PCI_read(bus, slot, func, 0x09);

	ret.revision = PCI_read(bus, slot, func, 0x08);
	ret.interrupt = PCI_read(bus, slot, func, 0x3C);

	return ret;
}

void PCI_loadDrivers() {
	for(int i=0; i<8; i++) {
		for(int j=0; j<32; j++) {
			int nFunctions = PCI_hasFunctions(i, j) ? 8 : 1;
			for(int k=0; k<nFunctions; k++) {
				struct PCI_DeviceDescriptor dd = PCI_getDD(i, j, k);
				if(dd.vendor_id == 0xFFFF) continue;

				/*switch(dd.class_id) {
					case 0x03:
						// Graphics.
						switch(dd.subclass_id) {
							case 0x00:
								// VGA.
								printf("VGA found!\n");
								break;
						}
						break;
				}*/

				printf("Found device -> %d.%d\n", dd.class_id, dd.subclass_id);
			}
		}
	}
}
