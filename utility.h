#pragma once
#include <cassert>

#ifdef _DEBUG
#define IFDEF_DEBUG_EXP(...) __VA_ARGS__
#else
#define IFDEF_DEBUG_EXP(...)
#endif

template<typename T, size_t D>
class ArrayView_impl final
{
public:
	const ArrayView_impl<T, D - 1> next;
	const size_t add;;
	IFDEF_DEBUG_EXP(const size_t size);
	template<typename... Args>
	constexpr ArrayView_impl(size_t size, size_t n, Args&&... args)noexcept
		: next(n, std::forward<Args>(args)...), add(next.add* n)
		IFDEF_DEBUG_EXP(, size(size))
	{ ((void*)&size); static_assert(sizeof...(Args) == D - 2); }

	constexpr ArrayView_impl(const ArrayView_impl& other) : next(other.next), add(other.add) IFDEF_DEBUG_EXP(, size(other.size)) {}
	template<typename = std::enable_if_t<!std::is_same_v<T, std::remove_const_t<T>>>>
	constexpr ArrayView_impl(const ArrayView_impl<std::remove_const_t<T>, D>& other) : next(other.next), add(other.add) IFDEF_DEBUG_EXP(, size(other.size)) {}
};
template<typename T>
class ArrayView_impl<T, 1> final
{
public:
	static constexpr size_t add = 1;
	IFDEF_DEBUG_EXP(const size_t size);
	constexpr ArrayView_impl(size_t size)noexcept
	IFDEF_DEBUG_EXP(: size(size))
	{ ((void*)(&size)); }

	ArrayView_impl(const ArrayView_impl& other) IFDEF_DEBUG_EXP(: size(other.size)) { ((void*)(&other));  }
	template<typename = std::enable_if_t<!std::is_same_v<T, std::remove_const_t<T>>>>
	ArrayView_impl(const ArrayView_impl<std::remove_const_t<T>, 1>& other) IFDEF_DEBUG_EXP(: size(other.size)) { ((void*)(&other)); }
};
template<typename T, size_t D>
class ArrayView_handler
{
private:
	T* const& data;
	const size_t index;
	const ArrayView_impl<T, D>& now;
public:
	constexpr ArrayView_handler(T* const& data, size_t index, const ArrayView_impl<T, D>& now)noexcept :data(data), index(index), now(now) {}

	constexpr ArrayView_handler<T, D - 1> operator[](size_t i)const noexcept {
		assert(i < now.size);
		return ArrayView_handler<T, D - 1>{data, index + i * now.add, now.next};
	}
};
template<typename T>
class ArrayView_handler<T, 1>
{
private:
	T* const& data;
	const size_t index;
	const ArrayView_impl<T, 1>& now;
public:
	constexpr ArrayView_handler(T* const& data, size_t index, const ArrayView_impl<T, 1>& now)noexcept :data(data), index(index), now(now) {}

	constexpr T& operator[](size_t i)const noexcept {
		assert(i < now.size);
		return data[index + i];
	}
};
template<typename T, size_t D>
class ArrayView final
{
private:
	T* const data;
	ArrayView_impl<T,D> impl;
	template<typename T_, size_t D_>
	friend class ArrayView;
public:
	template<typename... Args>
	constexpr ArrayView(T* data, Args&&... args)noexcept :data(data), impl(std::forward<Args>(args)...) { static_assert(sizeof...(Args) == D); }

	ArrayView(const ArrayView& other):data(other.data), impl(other.impl) {}
	template<typename=std::enable_if_t<!std::is_same_v<T, std::remove_const_t<T>>>>
	ArrayView(const ArrayView<std::remove_const_t<T>, D>& other) :data(other.data), impl(other.impl) {}

	constexpr ArrayView_handler<T, D - 1> operator[](size_t i)const noexcept {
		return ArrayView_handler<T, D>{data, 0, impl} [i] ;
	}
};


#include <iterator>
template<typename Integer>
struct range_iterator {
	using difference_type = Integer;
	using value_type = std::remove_cv_t<Integer>;
	using pointer = value_type*;
	using reference = value_type&;
	using iterator_category = std::random_access_iterator_tag;
	Integer value;
	constexpr range_iterator()noexcept {}
	constexpr range_iterator(Integer value)noexcept:value(value) {}
	Integer operator*() { return value; }
	range_iterator  operator++(int) noexcept { auto tmp = *this; ++value; return tmp; }
	range_iterator& operator++() noexcept { ++value; return *this; }
	range_iterator  operator--(int) noexcept { auto tmp = *this; --value; return tmp; }
	range_iterator& operator--() noexcept { --value; return *this; }
	range_iterator& operator+=(difference_type v) noexcept { value += v; return *this; }
	range_iterator& operator-=(difference_type v) noexcept { value -= v; return *this; }
	range_iterator operator+(difference_type v)const noexcept { auto tmp = *this; return tmp += v; }
	range_iterator operator-(difference_type v)const noexcept { auto tmp = *this; return tmp -= v; }
	difference_type operator-(const range_iterator& r)const noexcept { return value - r.value; }
	bool operator>(const range_iterator& r)const noexcept { return value > r.value; }
	bool operator<(const range_iterator& r)const noexcept { return value < r.value; }
	bool operator!=(const range_iterator& r)const noexcept { return value != r.value; }
	bool operator==(const range_iterator& r)const noexcept { return value == r.value; }
};