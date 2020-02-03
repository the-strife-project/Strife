JOTAFS is a simplified (and arguably worse) version of ext2.

Some aclarations:
- Everything is in Little-Endian.
- When I say «sector» I mean «physical block» (doesn't actually have to be a physical sector).
- The filesystem consists in the superblock, inodes, and blocks.
- The superblock contains information about the later three.
- An inode is an array of 128 bytes containing all metadata about a file but its name.
- A block is a sector containing raw data (the contents of a file).
- Bitmaps will be used in order to find free blocks. They are contiguous in disk.

The first sector contains the first stage of the bootloader.
The second sector is the superblock, which contains the following fields:

| Start byte | Size (bytes) | Description |
| --- | --- | --- |
| 0   | 8   | Signature (0x000CACADEBACA000) |
| 8   | 4   | Number of inodes |
| 12  | 4   | Number of blocks |
| 16  | 4   | ID of the first free inode |
| 20  | 4   | ID of the last free inode |
| 24  | 4   | Sector of the first block bitmap |
| 28  | 4   | ID of the first non-full bitmap |
| 32  | 4   | Sector of the first block |
| 36  | 476 | Unused |

Then, inodes. There are four of them in each sector, and the first inode is the number 1, as 0 is reserved for the null inode (necessary for the linked list). Their structure depends on the first byte. If it's 0x00, the inode is free. If it's 0x01, it's in use. This way, fields are overlapped.

If the inode is free, this is its structure:

| Start byte | Size (bytes) | Description |
| --- | --- | --- |
| 0   | 1   | Fixed to 0. |
| 1   | 3   | Padding. |
| 4   | 4   | Sector number of the next free inode. 0 if it's there isn't. |
| 8  | 120 | Padding to 128 bytes. |

This way, the free inodes form a singly linked list in disk, which behaves like a queue.

If it's in use, it's as follows:

| Start byte | Size (bytes) | Description |
| --- | --- | --- |
| 0   | 1   | Fixed to 1. |
| 1   | 3   | Padding. Unused. |
| 4   | 4   | Number of (hard) links. |
| 8   | 8   | File size (bytes) |
| 16  | 4   | Creation time (POSIX) |
| 20  | 4   | Last modification time (POSIX) |
| 24  | 4   | Last access time (POSIX) |
| 28  | 4   | Number of blocks the file uses |
| 32  | 4   | Direct Block Pointer (DBP) 0 |
| 36  | 4   | DBP 1 |
| 40  | 4   | DBP 2 |
| 44  | 4   | DBP 3 |
| 48  | 4   | DBP 4 |
| 52  | 4   | DBP 5 |
| 56  | 4   | DBP 6 |
| 60  | 4   | DBP 7 |
| 64  | 4   | DBP 8 |
| 68  | 4   | DBP 9 |
| 72  | 4   | Singly Indirect Block Pointer |
| 76  | 4   | Doubly Indirect Block Pointer |
| 80  | 4   | Triply Indirect Block Pointer |
| 84  | 4   | Quadruply Indirect Block Pointer |
| 88  | 4   | Owner's user ID |
| 92 | 2   | Permissions. See below. |
| 94 | 1   | Filetype. |
| 95 | 1   | Flags. |
| 96 | 32  | Padding to 128 bytes. |

The permissions bitmap is formed as folows:

| Bit (0 is MSB) | Contents |
| --- | --- |
| 0    | Owner read (ur) |
| 1    | Owner write (uw) |
| 2    | Owner execute (ux) |
| 3    | Group read (gr) |
| 4    | Group write (gw) |
| 5    | Group execute (gx) |
| 6    | Other read (or) |
| 7    | Other write (ow) |
| 8    | Other execute (ox) |
| 9-15 | Padding. Unused. |

The filetype:

| ID | Value |
| --- | --- |
| 0     | Regular file. |
| 1     | Directory. |
| 2     | FIFO (pipe). |
| 3     | Suction pipe. |
| 4     | Socket. |
| 5     | Volatile (in RAM). |
| 6     | System (regular) file. Cannot be deleted, and has a reserved inode number. |
| 7     | Soft link. |
| 8-255 | Unused. In case there's need for more. |

Flags:

| Bit (0 is MSB) | Contents |
| --- | --- |
| 0   | Fast link. If the filetype is soft link, the path overlaps the block pointers. |

Some inodes are reserved:

| Inode number | Contents |
| --- | --- |
| 1   | JBoot's second stage |
| 2   | The kernel |
| 3   | The root directory |

A directory is a file with its filetype set to 1. Its blocks contain contiguous file entries. Each entry is formed by the filename (max length being 507 bytes), made up of any characters greater than 31 but '/', terminated by a null byte, and 4 bytes specifying the inode number of that entry.

After the inodes, there's a multi-sector bitmap marking whether a block is in use (1) or free (0).

Finally, blocks. After the last sector of the bitmap, blocks start, containing raw data.
