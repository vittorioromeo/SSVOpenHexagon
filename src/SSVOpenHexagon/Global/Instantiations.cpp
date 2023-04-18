#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstddef>

template class std::vector<std::string>;

template class std::optional<int>;
template class std::optional<std::size_t>;
template class std::optional<std::string>;

template class std::unordered_map<std::string, float>;
template class std::unordered_map<float, std::string>;
template class std::unordered_map<std::string, std::string>;

template class std::unordered_set<std::string>;

template class std::function<void()>;
template class std::function<bool()>;
template class std::function<std::string()>;
