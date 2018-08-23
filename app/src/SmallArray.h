#ifndef SmallArray_h_
#define SmallArray_h_

#include <algorithm>

template <typename T, size_t PREALLOC>
class SmallArray {
public:
	SmallArray() : m_size(0), extra(NULL) { }
	SmallArray(const SmallArray &o) : m_size(o.m_size) {
		if (m_size) {
			if (m_size > PREALLOC) {
				std::copy(o.pre, o.pre + PREALLOC, pre);
				extra = new T[m_size - PREALLOC];
				std::copy(o.extra, o.extra + m_size - PREALLOC, extra);
			} else {
				std::copy(o.pre, o.pre + m_size, pre);
			}
		}
	}
	~SmallArray() {
		if (m_size > PREALLOC) delete[] extra;
	}
	SmallArray &operator=(const SmallArray &o) {
		if (&o == this) return *this;
		if (o.m_size) {
			if (o.m_size > PREALLOC) {
	 			std::copy(o.pre, o.pre + PREALLOC, pre);
				if (m_size != o.m_size) {
					delete[] extra;
					extra = new T[o.m_size - PREALLOC];
				}
				std::copy(o.extra, o.extra + o.m_size - PREALLOC, extra);
			} else {
				if (m_size > PREALLOC) delete[] extra;
				std::copy(o.pre, o.pre + o.m_size, pre);
			}
		} else if (m_size > PREALLOC) {
			delete[] extra;
		}
		m_size = o.m_size;
		return *this;
	}
	const T& operator[](size_t idx) const {
		return idx < PREALLOC ? pre[idx] : extra[idx - PREALLOC];
	}
	T& operator[](size_t idx) {
		return idx < PREALLOC ? pre[idx] : extra[idx - PREALLOC];
	}

	inline size_t size() const { return m_size; };
	inline bool isEmpty() const { return m_size == 0; }
	void clear() {
		if (m_size > PREALLOC) delete[] extra;
		m_size = 0;
	}
	void resize(size_t new_size) {
		if (m_size != new_size) {
			if (new_size > PREALLOC) {
				if (m_size > PREALLOC) {
					T *n = new T[new_size - PREALLOC];
					if (new_size > m_size) {
						std::copy(extra, extra + m_size - PREALLOC, n);
					} else {
						std::copy(extra, extra + new_size - PREALLOC, n);
					}
					delete[] extra;
					extra = n;
				} else {
					extra = new T[new_size - PREALLOC];
				}
			} else {
				if (m_size > PREALLOC) {
					delete[] extra;
				}
			}
		}
		m_size = new_size;
	}
private:
	size_t m_size;
	T* extra;
	T pre[PREALLOC];
};

#endif
