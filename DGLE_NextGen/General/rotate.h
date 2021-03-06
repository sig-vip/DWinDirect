/**
\author		Alexey Shaydurov aka ASH
\date		09.08.2016 (c)Korotkov Andrey

This file is a part of DGLE project and is distributed
under the terms of the GNU Lesser General Public License.
See "DGLE.h" for more details.
*/

#pragma once

#if !defined  __clang__  && defined _MSC_FULL_VER && _MSC_FULL_VER < 190023918
#error Old MSVC compiler version. Visual Studio 2015 Update 2 or later required.
#endif

#include <type_traits>
#include <limits>
#include <functional>
#include <cstdint>
#include <boost/integer.hpp>
#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace RotImpl
{
	using namespace std;

	template<unsigned width, typename Value>
	static constexpr auto Mask = (Value(1) << width) - Value(1);

	template<typename T, typename = void>
	struct Has_exact : false_type {};

	template<typename T>
	struct Has_exact<T, void_t<typename T::exact>> : true_type {};

	template<unsigned width>
	inline auto rol(typename boost::uint_t<width>::fast value, unsigned int shift) noexcept ->
		enable_if_t<
#ifdef _MSC_VER
			width != 8 && width != 16 && width != 32 && width != 64 &&
#endif
			!Has_exact<typename boost::uint_t<width>>::value, decltype(value)>
	{
		shift %= width;
		constexpr auto mask = Mask<width, decltype(value)>;
		return value & ~mask | (value << shift | (value & mask) >> width - shift) & mask;
	}

	template<unsigned width>
	inline auto rol(typename boost::uint_t<width>::exact value, unsigned int shift) noexcept ->
#ifdef _MSC_VER
		enable_if_t<width != 8 && width != 16 && width != 32 && width != 64, decltype(value)>
#else
		decltype(value)
#endif
	{
		shift %= width;
		return value << shift | value >> width - shift;
	}

	template<unsigned width>
	inline auto ror(typename boost::uint_t<width>::fast value, unsigned int shift) noexcept ->
		enable_if_t<
#ifdef _MSC_VER
			width != 8 && width != 16 && width != 32 && width != 64 &&
#endif
			!Has_exact<typename boost::uint_t<width>>::value, decltype(value)>
	{
		shift %= width;
		constexpr auto mask = Mask<width, decltype(value)>;
		return value & ~mask | ((value & mask) >> shift | value << width - shift) & mask;
	}

	template<unsigned width>
	inline auto ror(typename boost::uint_t<width>::exact value, unsigned int shift) noexcept ->
#ifdef _MSC_VER
		enable_if_t<width != 8 && width != 16 && width != 32 && width != 64, decltype(value)>
#else
		decltype(value)
#endif
	{
		shift %= width;
		return value >> shift | value << width - shift;
	}

#ifdef _MSC_VER
#define ROT_SPECIALIZATIONS(dir)																		\
	template<unsigned width>																			\
	inline enable_if_t<width == 8, typename function<decltype(_rot##dir##8)>::result_type> ro##dir(		\
		typename function<decltype(_rot##dir##8)>::first_argument_type value,							\
		typename function<decltype(_rot##dir##8)>::second_argument_type shift)							\
	{																									\
		return _rot##dir##8(value, shift);																\
	}																									\
																										\
	template<unsigned width>																			\
	inline enable_if_t<width == 16, typename function<decltype(_rot##dir##16)>::result_type> ro##dir(	\
		typename function<decltype(_rot##dir##16)>::first_argument_type value,							\
		typename function<decltype(_rot##dir##16)>::second_argument_type shift)							\
	{																									\
		return _rot##dir##16(value, shift);																\
	}																									\
																										\
	template<unsigned width>																			\
	inline enable_if_t<width == 32, typename function<decltype(_rot##dir)>::result_type> ro##dir(		\
		typename function<decltype(_rot##dir)>::first_argument_type value,								\
		typename function<decltype(_rot##dir)>::second_argument_type shift)								\
	{																									\
		return _rot##dir(value, shift);																	\
	}																									\
																										\
	template<unsigned width>																			\
	inline enable_if_t<width == 64, typename function<decltype(_rot##dir##64)>::result_type> ro##dir(	\
		typename function<decltype(_rot##dir##64)>::first_argument_type value,							\
		typename function<decltype(_rot##dir##64)>::second_argument_type shift)							\
	{																									\
		return _rot##dir##64(value, shift);																\
	}

	ROT_SPECIALIZATIONS(l)
	ROT_SPECIALIZATIONS(r)

#undef ROT_SPECIALIZATIONS
#endif

	enum class Dir
	{
		Left,
		Right,
	};

	template<Dir dir, unsigned width, typename Value, typename Shift>
	inline auto rot_dispatch(Value value, Shift shift) -> enable_if_t<dir == Dir::Left, make_unsigned_t<common_type_t<Value, decltype(rol<width>(value, shift))>>>
	{
		return rol<width>(value, shift);
	}

	template<Dir dir, unsigned width, typename Value, typename Shift>
	inline auto rot_dispatch(Value value, Shift shift) -> enable_if_t<dir == Dir::Right, make_unsigned_t<common_type_t<Value, decltype(rol<width>(value, shift))>>>
	{
		return ror<width>(value, shift);
	}

	template<Dir dir, unsigned width, typename Value, typename Shift>
	inline auto rot(Value value, Shift shift)
	{
		static_assert(is_integral_v<Value> && is_integral_v<Shift>, "rotate is feasible for integral types only");
		static_assert(width <= numeric_limits<uintmax_t>::digits, "too large width");
		return rot_dispatch<dir, width>(value, shift);
	}

	// auto does not work with VS 2015 Update 2/3 in cases such as 'const auto i = rol(1, 2);'
	template<typename Value>
	static constexpr /*auto*/size_t Width = numeric_limits<make_unsigned_t<Value>>::digits;
}

template<unsigned width, typename Value, typename Shift>
inline auto rol(Value value, Shift shift)
{
	return RotImpl::rot<RotImpl::Dir::Left, width>(value, shift);
}

template<unsigned width, typename Value, typename Shift>
inline auto ror(Value value, Shift shift)
{
	return RotImpl::rot<RotImpl::Dir::Right, width>(value, shift);
}

template<typename Value, typename Shift>
inline auto rol(Value value, Shift shift)
{
	return rol<RotImpl::Width<Value>>(value, shift);
}

template<typename Value, typename Shift>
inline auto ror(Value value, Shift shift)
{
	return ror<RotImpl::Width<Value>>(value, shift);
}