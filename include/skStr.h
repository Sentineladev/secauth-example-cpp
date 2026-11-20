#pragma once

#ifdef _KERNEL_MODE
namespace std {
template <class T> struct remove_reference { using type = T; };
template <class T> struct remove_reference<T&> { using type = T; };
template <class T> struct remove_reference<T&&> { using type = T; };
template <class T> using remove_reference_t = typename remove_reference<T>::type;
template <class T> struct remove_const { using type = T; };
template <class T> struct remove_const<const T> { using type = T; };
template <class T> using remove_const_t = typename remove_const<T>::type;
}
#else
#include <type_traits>
#endif

namespace skc {
template<class T> using clean_type = typename std::remove_const_t<std::remove_reference_t<T>>;
template<int Size, char Key1, char Key2, typename T>
class skCrypter {
public:
    __forceinline constexpr skCrypter(T* data) { crypt(data); }
    __forceinline T* get() { return storage_; }
    __forceinline int size() { return Size; }
    __forceinline char key() { return Key1; }
    __forceinline T* encrypt() { if (!isEncrypted()) crypt(storage_); return storage_; }
    __forceinline T* decrypt() { if (isEncrypted()) crypt(storage_); return storage_; }
    __forceinline bool isEncrypted() { return storage_[Size - 1] != 0; }
    __forceinline void clear() { for (int i = 0; i < Size; ++i) storage_[i] = 0; }
    __forceinline operator T*() { decrypt(); return storage_; }
private:
    __forceinline constexpr void crypt(T* data) {
        for (int i = 0; i < Size; ++i) storage_[i] = data[i] ^ (Key1 + i % (1 + Key2));
    }
    T storage_[Size]{};
};
}
#define skCrypt(str) skCrypt_key(str, __TIME__[4], __TIME__[7])
#define skCrypt_key(str, key1, key2) []() { \
    constexpr static auto crypted = skc::skCrypter<sizeof(str) / sizeof(str[0]), key1, key2, skc::clean_type<decltype(str[0])>>((skc::clean_type<decltype(str[0])>*)str); \
    return crypted; }()
