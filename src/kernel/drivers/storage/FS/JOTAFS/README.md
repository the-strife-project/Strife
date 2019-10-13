JOTAFS is a simplified version of ext2 that adapts to this OS in particular.

Some aclarations:
- Everything is in Little-Endian.
- The filesystem consists in the superblock, inodes, chunks, and blocks.
- The superblock contains information about the later three.
- An inode is a sector (512 bytes) containing all metadata about a file but the filename.
- A block is also a sector containing raw data (file contents).
- A chunk is a set of 512 sectors (256KiB), 511 of which are blocks.

The first sector contains the first stage of the bootloader.
The second sector is the superblock, which contains the following fields:

| Start byte | Size (bytes) | Description |
| --- | --- | --- |
| 0   | 8   | Signature (0x000CACADEBACA000) |
| 8   | 4   | Number of inodes |
| 12   | 4   | Number of chunks |
| 16  | 4   | First unallocated inode |
| 20  | 4   | LBA sector of the first inode (reserved: 2) |
| 24  | 4   | LBA sector of the first chunk |

From the third to the (3 + Number of inodes) sector, inodes are present. Each one is structured as follows:

| Start byte | Size (bytes) | Description |
| --- | --- | --- |
| 0   | 8   | File size (bytes) |
| 8   | 4   | Creation time (POSIX) |
| 12  | 4   | Last modification time (POSIX) |
| 16  | 4   | Last access time (POSIX) |
| 20  | 4   | Number of blocks the file uses |
| 24  | 4   | Direct Block Pointer (DBP) 0 |
| 28  | 4   | DBP 1 |
| 32  | 4   | DBP 2 |
| 36  | 4   | DBP 3 |
| 40  | 4   | DBP 4 |
| 44  | 4   | DBP 5 |
| 48  | 4   | DBP 6 |
| 52  | 4   | DBP 7 |
| 56  | 4   | DBP 8 |
| 60  | 4   | DBP 9 |
| 64  | 4   | Singly Indirect Block Pointer |
| 68  | 4   | Doubly Indirect Block Pointer |
| 72  | 4   | Triply Indirect Block Pointer |
| 76  | 4   | Quadruply Indirect Block Pointer |
| 80  | 4   | User ID |
| 84  | 1   | File is an app (executable), either 0 or 1 |
| 85  | 1   | File is a directory, 0 or 1 |
| 86  | 1   | Whether this inode is in use, 0 or 1 |

Some inodes are reserved.

| Inode number | Contents |
| --- | --- |
| 0   | JBoot's second stage |
| 1   | The kernel |
| 2   | The root directory |

A directory is a file with its inode's 85th byte set to 1. Its blocks contain file entries (which have no separators between). Each entry is formed by the filename (max length being 507 bytes), made up of any characters greater than 31 but ':' (58) and terminated with a null byte, and 4 bytes specifying the inode number of that entry.

The remaining sectors are chunks. Each one is 512 sectors long. The first sector of a chunk is a "bytemap". The first byte indicates whether the remaining bytes are all 1. As for the rest, the nth byte marks whether the nth block of the chunk is free (0) or in use (1). The other 511 sectors are blocks, that is, raw data.
