#pragma once
#include "Allocator.h"

template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = 
		//static_cast<Type*>(BaseAllocator::Alloc((sizeof(Type))));
		static_cast<Type*>(Xalloc(sizeof(Type)));

	// placement new, variable args
	new(memory) Type(std::forward<Args>(args)...);

	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	//BaseAllocator::Release(obj);
	Xrelease(obj);
}