#ifndef BITMAP_H
#define BITMAP_H

class bitmap {
private:
	size_t sz;
	unsigned char* data;

public:
	bitmap(size_t sz, unsigned char* data) : sz(sz), data(data) {}

	void set(size_t idx, bool v) {
		unsigned char* aux = data + (idx/8);
		idx %= 8;
		if(v) *aux |= (1 << (7 - idx));
		else *aux &= ~(1 << (7 - idx));
	}

	bool get(size_t idx) const {
		unsigned char aux = *(data + (idx/8));
		idx %= 8;
		aux &= (1 << (7 - idx));
		return (aux != 0);
	}

	size_t getFirstZero() const {
		for(size_t i=0; i<sz; i++)
			if(!get(i))
				return i;
		return sz;	// Impossible value.
	}

	size_t getFirstZeroAndFlip() {
		size_t ret = getFirstZero();
		if(ret != sz) set(ret, true);
		return ret;
	}

	bool getAllOne() const {
		for(size_t i=0; i<(sz/8); i++)
			if(!data[i])
				return false;
		return true;
	}
};

#endif
