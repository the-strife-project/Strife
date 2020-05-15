#include <kernel/mounts/mounts.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/STL/dlist>

// Mount points.
static map<string, VFS*, greater<pair<string, VFS*>>> _mounts;

bool startsWith(const string& str, const string& substr) {
	if(str.length() < substr.length())
		return false;

	auto str_it = str.begin();
	auto substr_it = substr.begin();
	while(substr_it != substr.end()) {
		if(*str_it != *substr_it)
			return false;

		++str_it;
		++substr_it;
	}

	return true;
}

pair<string, VFS*> getMount(const string& path) {
	for(auto const& x : _mounts)
		if(startsWith(path, x.f))
			return x;

	printf(" {{ jlxip's fault: getMount assumptions failed. }} ");
	while(true) {}
	return pair<string, VFS*>(string(), nullptr);
}

// Local function, should not be needed outside. Hooray for abstraction!
// "/mnt/cd/bin/" -> "/bin/".
// Also returns the VFS for further operations.
pair<string, VFS*> getInnerPath(const string& path) {
	pair<string, VFS*> mount = getMount(path);
	VFS* vfs = mount.s;
	string mount_point = mount.f;

	string inner_path;
	for(uint32_t i=mount_point.length(); i<path.length(); ++i)
		inner_path += path[i];

	inner_path.appendb("/");	// Add a '/' at the beginning.
	return pair<string, VFS*>(inner_path, vfs);
}

// "/sys/../bin/" -> "/bin/"
// How do I call this? shortenPath is the only thing that pops in mind.
string shortenPath(const string& str) {
	list<string> _parts(str.split('/'));

	dlist<string> parts;
	for(auto const& x : _parts) {
		if(x == "" || x == ".")
			continue;

		if(x == "..") {
			if(!parts.empty())
				parts.pop_back();
		} else {
			parts.push_back(x);
		}
	}

	string ret("/");
	for(auto const& x : parts) {
		ret += x;
		ret += '/';
	}

	if(ret.length() > 1)
		ret.pop_back();	// Remove last '/', could be a file.

	return ret;
}

// Resolves symbolic links and shortens path and such.
// Returns the VFS as well.
pair<string, VFS*> getActualPath(string path) {
	path = shortenPath(path);
	pair<string, VFS*> inner = getInnerPath(path);

	while(inner.s->isSymLink(inner.f, path)) {
		path = shortenPath(path);
		inner = getInnerPath(path);
		inner.f = shortenPath(inner.f);	// Yes, again.
	}
	return inner;
}

void mountRoot(VFS* vfs) {
	_mounts.insert(pair<string, VFS*>("/", vfs));
}

void mount(const string& path, VFS* vfs) {
	_mounts.insert(pair<string, VFS*>(getActualPath(path).f, vfs));
}

void umount(const string& path) {
	_mounts.erase(pair<string, VFS*>(getActualPath(path).f, _mounts[path]));
}

FSRawChunk readFile(const string& path, bool big) {
	pair<string, VFS*> actual(getActualPath(path));
	return actual.s->readFile(actual.f, big);
}

void mkd(const string& path) {
	pair<string, VFS*> actual(getActualPath(path));
	actual.s->newdir(actual.f);
}

set<string> ls(const string& path) {
	pair<string, VFS*> actual(getActualPath(path));
	return actual.s->ls(actual.f);
}

void link(const string& orig, const string& dst) {
	// dst remains unchanged, of course.
	pair<string, VFS*> actual(getActualPath(orig));
	return actual.s->link(actual.f, dst);
}

bool isDir(const string& path) {
	pair<string, VFS*> actual(getActualPath(path));
	return actual.s->isDir(actual.f);
}

bool isFile(const string& path) {
	pair<string, VFS*> actual(getActualPath(path));
	return actual.s->isFile(actual.f);
}

void copy(const string& orig, const string& dst) {
	FSRawChunk contents(readFile(orig, false));

	string short_dst = shortenPath(dst);
	pair<string, VFS*> inner_dst = getInnerPath(short_dst);
	inner_dst.s->newfile(inner_dst.f, contents);

	contents.destroy();
}
