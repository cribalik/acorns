#ifndef ARRAY_HPP
#define ARRAY_HPP

#ifndef ARRAY_INITIAL_SIZE
  #define ARRAY_INITIAL_SIZE 4
#endif

#ifndef ARRAY_REALLOC
  #include <stdlib.h>
  #define ARRAY_REALLOC realloc
#endif

#ifndef ARRAY_FREE
  #include <stdlib.h>
  #define ARRAY_FREE free
#endif

template<class T>
struct Array {
	T *data;
	int size,cap;

	T& operator[](int i) {return data[i];}
	const T& operator[](int i) const {return data[i];}
	operator T*() {return data;}
};

template<class T>
T* array_last(Array<T> a) {
	return a.data+a.size-1;
}

template<class T>
void array_push(Array<T> &a, T val) {
	array_pushn(a, 1);
	a.data[a.size-1] = val;
}

template<class T>
void array_insertz(Array<T> &a, int i) {
	array_insert(a, i, T());
}

template<class T>
void array_insert(Array<T> &a, int i, T value) {
	array_pushn(a, 1);
	memmove(a.data+i+1, a.data+i, (a.size-i)*sizeof(T));
	a.data[i] = value;
}

template<class T>
void array_resize(Array<T> &a, int newsize) {
	if (newsize > a.size)
		array_pushn(a, newsize-a.size);
	a.size = newsize;
}

template<class T>
void array_reserve(Array<T> &a, int size) {
	int oldsize = a.size;
	if (size > a.size)
		array_pushn(a, size-a.size);
	a.size = oldsize;
}

template<class T>
void array_push(Array<T> &a, T *data, int n) {
	array_pushn(a, n);
	memmove(a.data+a.size-n, data, n*sizeof(T));
}

template<class T>
void array_remove(Array<T> &a, int i) {
	a.data[i] = a.data[a.size-1];
	--a.size;
}

template<class T>
void array_remove_slown(Array<T> &a, int i, int n) {
	memmove(a.data+i, a.data+i+n, (a.size-i-n)*sizeof(T));
	a.size -= n;
}

template<class T>
void array_remove_slow(Array<T> &a, int i) {
	memmove(a.data+i, a.data+i+1, (a.size-i-1)*sizeof(T));
	--a.size;
}

template<class T>
void array_insertn(Array<T> &a, int i, int n) {
	array_pushn(a, n);
	memmove(a.data+i+n, a.data+i, (a.size-i-n)*sizeof(T));
}

template<class T>
void array_pushz(Array<T> &a) {
	array_pushn(a, 1);
	a.data[a.size-1] = T();
}

template<class T>
void array_inserta(Array<T> &a, int i, const T *data, int n) {
	array_pushn(a, n);
	memmove(a.data+i+n, a.data+i, (a.size-i-n)*sizeof(T));
	memcpy(a.data+i, data, n*sizeof(T));
}

template<class T>
T* array_pushn(Array<T> &a, int n) {
	if (a.size+n >= a.cap) {
		int newcap = a.cap ? a.cap*2 : 1;
		while (newcap < a.size+n)
			newcap *= 2;
		a.data = (T*)ARRAY_REALLOC(a.data, newcap * sizeof(T));
		a.cap = newcap;
	}
	a.size += n;
	return a.data + a.size - n;
}

template<class T>
void array_pusha(Array<T> &a, T *data, int n) {
	array_pushn(a, n);
	memcpy(a.data+a.size-n, data, n*sizeof(T));
}

template<class T>
void array_free(Array<T> &a) {
	if (a.data)
		ARRAY_FREE(a.data);
	a.data = 0;
	a.size = a.cap = 0;
}

#ifndef array_find
#define array_find(a, ptr, expr) { \
	for ((ptr) = (a).data; (ptr) < (a).data+(a).size; ++(ptr)) \
		if (expr) \
			break; \
	if ((ptr) == (a).data+(a).size) \
	  (ptr) = 0; \
}
#endif /*array_find*/

#ifndef array_foreach
#define array_foreach(a, ptr) for ((ptr) = (a).data; (ptr) && (ptr) < (a).data+(a).size; ++(ptr))
#endif /*array_foreach*/

#ifndef array_copy
#define array_copy(a, dest) memcpy(dest, (a).data, (a).size*sizeof(*(a).data))
#endif /*array_copy*/

#endif