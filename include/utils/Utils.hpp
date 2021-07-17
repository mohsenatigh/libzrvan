#pragma once
#include <cstdint>
#include <string>
namespace libzrvan {
namespace utils {
class Utils {
 public:
  //-------------------------------------------------------------------------------------
  /**
   * @brief 
   * 
   * @param len 
   * @return std::string 
   */
  static std::string generateRandomString(uint32_t len) {
      std::string out;
      out.reserve(len);
      for(uint32_t i=0;i<len;i++){
          char rndNum=(rand()%93)+32;
            out.push_back(rndNum);
      }
      return out;
  }
};
}  // namespace utils
}  // namespace libzrvan
