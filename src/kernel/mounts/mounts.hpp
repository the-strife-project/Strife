#ifndef MOUNTS_HPP
#define MOUNTS_HPP

#include <kernel/klibc/STL/map>
#include <kernel/drivers/storage/FS/VFS.hpp>

// Assumes there is a mount point in path or upper.
pair<string, VFS*> getMount(const string& path);

/* ABSTRACT FS UTILITIES */
string shortenPath(const string& str);
void mountRoot(VFS* vfs);
void mount(const string& path, VFS* vfs);
void umount(const string& path);
FSRawChunk readFile(const string& path, bool big);
void mkd(const string& path);
set<string> ls(const string& path);
void link(const string& orig, const string& dst);
bool isDir(const string& path);
bool isFile(const string& path);
void copy(const string& orig, const string& dst);

#endif
