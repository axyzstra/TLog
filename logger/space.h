#pragma

#include <iostream>
#include <ratio>

namespace logger {
template <class _Rep, class _Capacity = std::ratio<1>>
class space;

/// @brief 内存单位 space 转换为指定单位
/// @tparam _ToSpace
/// @tparam _Rep
/// @tparam _Capacity
/// @param __d
/// @return
template <typename _ToSpace, typename _Rep, typename _Capacity>
constexpr _ToSpace space_cast(const space<_Rep, _Capacity>& __d) {
  auto __r = __d.count() * std::ratio_divide<_Capacity, typename _ToSpace::capacity>::num /
             std::ratio_divide<_Capacity, typename _ToSpace::capacity>::den;
  return _ToSpace(__r);
}

template <typename _Rep, typename _Capacity>
class space {
 public:
  using rep = _Rep;
  using capacity = _Capacity;

  constexpr space() : _rep() {}

  template <typename _Rep2>
  explicit constexpr space(const _Rep2& __r) : _rep(__r) {}

  template <typename _Rep2, typename _Capacity2>
  constexpr space(const space<_Rep2, _Capacity2>& __d) : _rep(space_cast<space<_Rep, _Capacity>>(__d).count()) {}

  constexpr _Rep count() const { return _rep; }

  constexpr space operator+() const { return _rep; }

  constexpr space operator-() const { return -(_rep); }

  /// @brief 后缀自增 space++
  /// @return
  space& operator++() {
    ++_rep;
    return *this;
  }

  space operator++(int) {
    space tmp(_rep);
    _rep++;
    return tmp;
  }

  space& operator-() {
    --_rep;
    return *this;
  }

  space operator-(int) {
    space tmp(_rep);
    _rep--;
    return tmp;
  }

  space& operator+=(const space& __d) {
    _rep += __d.count();
    return *this;
  }

  space& operator-=(const space& __d) {
    _rep -= __d.count();
    return *this;
  }

  space& operator*=(const _Rep& __s) {
    _rep *= __s;
    return *this;
  }

  space& operator/=(const _Rep& __s) {
    _rep /= __s;
    return *this;
  }

  space& operator%=(const _Rep& __s) {
    _rep %= __s;
    return *this;
  }

 private:
  rep _rep;
};

template <typename _Rep, typename _Capacity>
constexpr space<_Rep, _Capacity> operator+(const space<_Rep, _Capacity>& __x, const space<_Rep, _Capacity>& __y) {
  return space<_Rep, _Capacity>(__x.count() + __y.count());
}

template <typename _Rep, typename _Capacity>
constexpr space<_Rep, _Capacity> operator-(const space<_Rep, _Capacity>& __x, const space<_Rep, _Capacity>& __y) {
  return space<_Rep, _Capacity>(__x.count() - __y.count);
}

using kilo = std::ratio<1024L, 1>;
using mega = std::ratio<1024L * 1024, 1>;
using giga = std::ratio<1024L * 1024 * 1024, 1>;
using tera = std::ratio<1024L * 1024 * 1024 * 1024, 1>;

using bytes = space<size_t>;
using kilobytes = space<size_t, kilo>;
using megabytes = space<size_t, mega>;
using gigabytes = space<size_t, giga>;
using terabytes = space<size_t, tera>;

}  // namespace logger