#pragma once

template <typename T>
class StlAllocator
{
public:
	typedef	T					value_type;
	typedef	value_type*			pointer;
	typedef const value_type*	const_pointer;
	typedef value_type&			reference;
	typedef const value_type&	const_reference;
	typedef std::size_t			size_type;
	typedef std::ptrdiff_t		difference_type;

	template <typename U>
	struct rebind { typedef StlAllocator<U> other; };

	inline	pointer			address(reference val) const { return &val; }
	inline	const_pointer	address(const_reference val) const { return val; }

	StlAllocator() {}
	StlAllocator(const StlAllocator<T>&) {}

	template <typename U>
	StlAllocator(const StlAllocator<U>&) {}
	~StlAllocator() {}

	template<typename U>
	StlAllocator<T>& operator =(const StlAllocator<U>&) { return *this; }

	template<typename _T>	bool operator == (const StlAllocator<_T>&) { return false; }
	template<typename _T>	bool operator != (const StlAllocator<_T>& rhs) { return true; }
	template<>	bool operator == (const StlAllocator<T>&) { return true; }
	template<>	bool operator != (const StlAllocator<T>&) { return false; }

	inline	pointer	allocate(size_type count)
	{
		return (pointer)malloc(count * sizeof(T));
	}
	inline	void	deallocate(pointer ptr, size_type size)
	{
		free(ptr);
	}

	inline	pointer	allocate(size_type count, const void*) { return allocate(count); }
	inline	void construct(pointer ptr, const value_type& val) { new(ptr) value_type(val); }
	inline	void destroy(pointer ptr) { ptr->~value_type(); }

	inline	size_type	max_size()	const
	{
		size_t	count = static_cast<size_type>(-1) / sizeof(T);
		return (0 < count) ? count : 1;
	}
};