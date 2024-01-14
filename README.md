# Smart Pointers

This repository contains an implementation of standard C++ smart pointers. The implementation uses various optimizations, such as EBO in unique_ptr. The code is well covered with unit tests.

## Types

* ```UniquePtr``` provides exclusive ownership of an object.
* ```SharedPtr```  provides shared ownership of an object.
* ```WeakPtr``` provides a non-owning pointer to an object managed by a `SharedPtr`. It's usually used to avoid cyclic references in `SharedPtr`.
* ```IntrusivePtr``` is a light-weight version of `SharedPtr` that can be used if the class of the object satisfies some requirements.
