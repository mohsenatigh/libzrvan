#pragma once

#include <cstdint>
#include <functional>
namespace libzrvan {
namespace utils {

/**
 * @brief 
 *
 */
class Defer {
 private:
  using DeferFunc = std::function<void()>;
  DeferFunc func_ = nullptr;

 public:
  Defer(DeferFunc func) { func_ = func; }
  ~Defer() { func_(); }
};

}  // namespace utils
}  // namespace libzrvan