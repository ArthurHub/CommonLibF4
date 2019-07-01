#pragma once

#include "f4se/GameAPI.h"

#include <cstdlib>
#include <new>


namespace RE
{
	inline void* malloc(std::size_t a_count)
	{
		return ::Heap_Allocate(a_count);
	}


	template <class T>
	inline T* malloc(std::size_t a_count)
	{
		return static_cast<T*>(malloc(a_count));
	}


	template <class T>
	inline T* malloc()
	{
		return static_cast<T*>(malloc(sizeof(T)));
	}


	inline void* calloc(std::size_t a_num, std::size_t a_size)
	{
		return malloc(a_num * a_size);
	}


	template <class T>
	inline T* calloc(std::size_t a_num, std::size_t a_size)
	{
		return static_cast<T*>(calloc(a_num, a_size));
	}


	template <class T>
	inline T* calloc(std::size_t a_num)
	{
		return static_cast<T*>(calloc(a_num, sizeof(T)));
	}


	inline void free(void* a_ptr)
	{
		::Heap_Free(a_ptr);
	}
}


#define TES_HEAP_REDEFINE_NEW()																										\
	void*	operator new(std::size_t a_count)													{ return RE::malloc(a_count); }		\
	void*	operator new[](std::size_t a_count)													{ return RE::malloc(a_count); }		\
	void*	operator new([[maybe_unused]] std::size_t a_count, void* a_plcmnt)					{ return a_plcmnt; }				\
	void*	operator new[]([[maybe_unused]] std::size_t a_count, void* a_plcmnt)				{ return a_plcmnt; }				\
	void	operator delete(void* a_ptr)														{ if (a_ptr) { RE::free(a_ptr); } }	\
	void	operator delete[](void* a_ptr)														{ if (a_ptr) { RE::free(a_ptr); } }	\
	void	operator delete([[maybe_unused]] void* a_ptr, [[maybe_unused]] void* a_plcmnt)		{ return; }							\
	void	operator delete[]([[maybe_unused]] void* a_ptr, [[maybe_unused]] void* a_plcmnt)	{ return; }
