#include <kernel/drivers/storage/FS/controllers/common.hpp>

pair<string, string> _common_cutlast(const string& path) {
	queue<string> parts;
	for(auto const& x : path.split('/'))
		if(x != "")
			parts.push(x);

	string parentPath("/");
	while(parts.size() > 1) {
		parentPath += parts.front();
		parentPath += '/';
		parts.pop();
	}

	return pair<string, string>(parentPath, parts.front());
}
