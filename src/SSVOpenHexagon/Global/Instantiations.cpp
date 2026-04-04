#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


template class std::vector<std::string>;

template class sf::base::Optional<int>;
template class sf::base::Optional<sf::base::SizeT>;
template class sf::base::Optional<std::string>;

template class std::unordered_map<std::string, float>;
template class std::unordered_map<float, std::string>;
template class std::unordered_map<std::string, std::string>;

template class std::unordered_set<std::string>;

template class std::function<void()>;
template class std::function<bool()>;
template class std::function<std::string()>;
