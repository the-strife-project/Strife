#ifndef ATA_PIO_H
#define ATA_PIO_H

#include <common/types.h>

#define BYTES_PER_SECTOR 512

class ATA {
private:
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
	void init(uint8_t master, uint16_t portBase);

public:
	ATA();
	ATA(uint8_t master, uint16_t portBase);
	ATA(const ATA& other);
	ATA& operator=(const ATA& other);

	uint8_t identify(uint16_t* retdata);
	uint8_t* read28(uint32_t sector);
	uint8_t write28(uint32_t sector, uint8_t* contents);
	uint8_t clear28(uint32_t sector);
};

#endif
