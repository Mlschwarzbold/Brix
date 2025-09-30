
#include <unordered_map>
namespace extensions {

template <typename Key, typename Tp>
class hash_map : public std::unordered_map<Key, Tp> {
  public:
    Tp try_get(Key position);
};
} // namespace extensions