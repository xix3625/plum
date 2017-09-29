#pragma once

template<typename T>
class IntrusivePtr
{
private:
	typedef IntrusivePtr this_type;

public:
	IntrusivePtr() : _px(nullptr) {}

	IntrusivePtr(T* p, bool add_ref = true) : _px(p)
	{
		if (_px != nullptr && add_ref) intrusive_ptr_add_ref(_px);
	}

	template<typename U>
	IntrusivePtr(IntrusivePtr<U> const& rhs) : _px(rhs.get())
	{
		if (_px != nullptr) intrusive_ptr_add_ref(_px);
	}

	IntrusivePtr(IntrusivePtr const& rhs) : _px(rhs.px)
	{
		if (_px != nullptr) intrusive_ptr_add_ref(_px);
	}

	~IntrusivePtr()
	{
		if (_px != nullptr) intrusive_ptr_release(_px);
	}

	template<typename U> 
	IntrusivePtr& operator=(IntrusivePtr<U> const& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr& operator=(IntrusivePtr const& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	IntrusivePtr& operator=(T* rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	void reset()
	{
		this_type().swap(*this);
	}

	void reset(T* rhs)
	{
		this_type(rhs).swap(*this);
	}

	T * get() const
	{
		return px;
	}

	T & operator*() const
	{
		BOOST_ASSERT(px != 0);
		return *px;
	}

	T * operator->() const
	{
		BOOST_ASSERT(px != 0);
		return px;
	}

	void swap(IntrusivePtr& rhs)
	{
		T * tmp = px;
		px = rhs.px;
		rhs.px = tmp;
	}

private:
	T*	_px;
};

template<typename T, typename U> 
inline bool operator==(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b)
{
	return a.get() == b.get();
}

template<typename T, typename U> 
inline bool operator!=(IntrusivePtr<T> const& a, IntrusivePtr<U> const& b)
{
	return a.get() != b.get();
}

template<typename T, typename U> 
inline bool operator==(IntrusivePtr<T> const& a, U* b)
{
	return a.get() == b;
}

template<typename T, typename U> 
inline bool operator!=(IntrusivePtr<T> const& a, U* b)
{
	return a.get() != b;
}

template<typename T, typename U> 
inline bool operator==(T* a, IntrusivePtr<U> const& b)
{
	return a == b.get();
}

template<typename T, typename U> 
inline bool operator!=(T* a, IntrusivePtr<U> const& b)
{
	return a != b.get();
}
