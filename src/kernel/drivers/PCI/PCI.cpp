#include <kernel/drivers/PCI/PCI.hpp>
#include <kernel/asm.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/paging/paging.hpp>

uint32_t __PCI_getAddress(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
	return (uint32_t)(
		(bus << 16) |
		(slot << 11) |
		(func << 8) |
		(offset & 0xFC) |
		((uint32_t)0x80000000)
	);
}

void PCI::prepare(uint8_t offset) {
	// Mark the page of the address as present.
	uint32_t addr = __PCI_getAddress(bus, slot, func, offset);
	paging_setPresent(addr);	// TODO: Just one?
	outl(PCI_CONFIG_PORT, __PCI_getAddress(bus, slot, func, offset));
}

uint16_t PCI::read(uint8_t offset) {
	prepare(offset);
	return (uint16_t)(inl(PCI_DATA_PORT) >> ((offset & 2) * 8));
}

bool PCI::exists() {
	return (desc0.vendorID != 0xFFFF);
}

PCI::PCI() {}

PCI::PCI(uint8_t bus, uint8_t slot, uint8_t func)
	: bus(bus), slot(slot), func(func)
{
	uint16_t* ptr = (uint16_t*)&desc0;
	for(uint8_t i=0; i<0x20; ++i)
		*(ptr++) = read(2*i);
}

uint8_t PCI::getBus() { return bus; }
uint8_t PCI::getSlot() { return slot; }
uint8_t PCI::getFunc() { return func; }

PCI::Descriptor0 PCI::getDescriptor() {
	return desc0;
}


// PROBING
list<PCI> PCI_probe() {
	list<PCI> ret;

	for(uint16_t bus=0; bus<256; ++bus) {
		for(uint8_t device=0; device<32; ++device) {
			PCI pci(bus, device, 0);
			if(pci.exists()) {
				if(pci.getDescriptor().headerType & (1 << 7)) {
					// Has functions.
					for(uint8_t function=0; function<8; ++function) {
						pci = PCI(bus, device, function);
						if(pci.exists())
							ret.push_back(pci);
					}
				} else {
					// Does not have functions.
					ret.push_back(pci);
				}
			}
		}
	}

	return ret;
}
