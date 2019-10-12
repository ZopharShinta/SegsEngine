#pragma once

namespace eastl {
    template <typename T, typename Allocator>
    class vector;
    template <class T, class A>
    class list;
    template <class K,class CMP, class A>
    class set;
}

template <class T>
class Ref;

template <class T>
class PoolVector;

class wrap_allocator;
class DefaultAllocator;

template <class T>
struct Comparator;

template<class T>
using PODVector = eastl::vector<T,wrap_allocator>;
template <class T>
class Vector;

template <class T, class A>
class List;

template <class T>
using DefList = class List<T,DefaultAllocator>;
template<class T>
using ListPOD = eastl::list<T,wrap_allocator>;
template <class T>
using Set = eastl::set<T, Comparator<T>, wrap_allocator>;