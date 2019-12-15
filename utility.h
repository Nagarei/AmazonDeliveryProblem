#pragma once

template<typename T, size_t D>
class ArrayView final
{
private:
	template<typename T, size_t D>
	class ArrayView_impl final
	{
	public:
		const ArrayView_impl<T, D - 1> next;
		const size_t add;
		template<typename... Args>
		constexpr ArrayView_impl(size_t n, Args&&... args)noexcept :next(std::forward<Args>(args)...), add(next.add * n) { static_assert(sizeof...(Args) == D - 2); }
	};
	template<typename T>
	class ArrayView_impl<T, 1> final
	{
	public:
		static constexpr size_t add = 1;
		constexpr ArrayView_impl()noexcept {}
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
			return ArrayView_handler<T, D - 1>{data, index + i * now.add, now.next};
		}
	};
	template<typename T>
	class ArrayView_handler<T, 1>
	{
	private:
		T* const& data;
		const size_t index;
	public:
		constexpr ArrayView_handler(T* const& data, size_t index, const ArrayView_impl<T, 1>&)noexcept :data(data), index(index) {}

		constexpr T& operator[](size_t i)const noexcept {
			return data[index + i];
		}
	};

	T* const data;
	ArrayView_impl<T,D> impl;
public:
	template<typename... Args>
	constexpr ArrayView(T* data, size_t, Args&&... args)noexcept :data(data), impl(std::forward<Args>(args)...) { static_assert(sizeof...(Args) == D - 1); }

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