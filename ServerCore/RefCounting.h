#pragma once

class RefCountable {
public:
	RefCountable() : _refCount(1) {}
	virtual ~RefCountable() {}

	int32 GetRefCount() { return _refCount; }

    int32 AddRef() { return ++_refCount; }

    int32 ReleaseRef()
    {
        int32 refCount = --_refCount;
        if (refCount == 0) {
            delete this;
        }
        return refCount;
    }

protected:
	std::atomic<int32> _refCount;
};


template<typename T>
class TSharedPtr {
    T* _ptr = nullptr;

public:
    TSharedPtr() {}
    TSharedPtr(T* ptr) { Set(ptr); }

    TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }
    TSharedPtr(TSharedPtr&& rhs)
    {
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
    }

    template <typename U>
    TSharedPtr(const TSharedPtr<U>& rhs)
    {
        Set(static_cast<T*>(rhs._ptr));
    }

    ~TSharedPtr() { Release(); }

    TSharedPtr& operator=(const TSharedPtr& rhs)
    {
        if (_ptr == rhs._ptr)
            return *this;

        Release();
        Set(rhs._ptr);
        return *this;
    }

    TSharedPtr& operator=(TSharedPtr&& rhs)
    {
        Release();
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
        return *this;
    }

    bool operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
    bool operator==(T* ptr) const { return _ptr == ptr; }
    bool operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
    bool operator!=(T* ptr) const { return _ptr != ptr; }
    bool operator<(const TSharedPtr& rhs) const { return _ptr < rhs._ptr; }
    T* operator*() { return _ptr; }
    const T* operator*() const { return _ptr; }
    operator T* () const { return _ptr; }
    T* operator->() { return _ptr; }
    const T* operator->() { return _ptr; }

    bool IsNull() { return _ptr == nullptr; }

private:
    inline void Set(T* ptr) {
        _ptr = ptr;
        if (ptr)
            ptr->AddRef();
    }

    inline void Release()
    {
        if (_ptr != nullptr) {
            _ptr->ReleaseRef();
            _ptr = nullptr;
        }
    }
};