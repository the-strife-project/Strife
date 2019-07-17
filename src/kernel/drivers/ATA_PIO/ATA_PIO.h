#ifndef ATA_PIO_H
#define ATA_PIO_H

#include <common/types.h>

#define BYTES_PER_SECTOR 512

struct ATA_INTERFACE {
	uint8_t master;
	uint16_t dataPort;
	uint16_t errorPort;
	uint16_t sectorCountPort;
	uint16_t lbaLowPort;
	uint16_t lbaMidPort;
	uint16_t lbaHiPort;
	uint16_t devicePort;
	uint16_t commandPort;
	uint16_t controlPort;
};

void expensiveNOP();

struct ATA_INTERFACE* newATA(uint8_t master, uint16_t portBase);
uint8_t ATA_identify(struct ATA_INTERFACE* iface);
uint8_t* ATA_read28(struct ATA_INTERFACE* iface, uint32_t sector);
void ATA_write28(struct ATA_INTERFACE* iface, uint32_t sector, uint8_t* contents);
void ATA_flush(struct ATA_INTERFACE* iface);

#endif
