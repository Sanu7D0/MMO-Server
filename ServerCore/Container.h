#pragma once

#include "Types.h"
#include "Allocator.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename Type>
using xvector = std::vector<Type, StlAllocator<Type>>;

template<typename Type>
using xlist = std::list<Type, StlAllocator<Type>>;

template<typename Key, typename Type, typename Pred = std::less<Key>>
using xmap = std::map<Key, Type, Pred, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key, typename Type, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using xunordered_map = std::unordered_map<Key, Type, Hasher, KeyEq, StlAllocator<std::pair<const Key, Type>>>;

template<typename Key, typename Hasher = std::hash<Key>, typename KeyEq = std::equal_to<Key>>
using xunordered_set = std::unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

template<typename Key, typename Pred = std::less<Key>>
using xset = std::set<Key, Pred, StlAllocator<Key>>;

template<typename Type>
using xdeque = std::deque<Type, StlAllocator<Type>>;

template<typename Type, typename Container = xdeque<Type>>
using xqueue = std::queue<Type, Container>;

template<typename Type, typename Container = xdeque<Type>>
using xstack = std::stack<Type, Container>;

template<typename Type, typename Container = xvector<Type>, typename Pred = std::less<typename Container::value_type>>
using xpriority_queue = std::priority_queue<Type, Container, Pred>;

using xstring = std::basic_string<char, std::char_traits<char>, StlAllocator<char>>;
using xwstring = std::basic_string<wchar_t, std::char_traits<wchar_t>, StlAllocator<wchar_t>>;