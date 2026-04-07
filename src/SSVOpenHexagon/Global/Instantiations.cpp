#include "SSVOpenHexagon/Global/StringHash.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>


template class sf::base::Vector<sf::base::String>;

template class sf::base::Optional<int>;
template class sf::base::Optional<sf::base::SizeT>;
template class sf::base::Optional<sf::base::String>;

template class std::unordered_map<sf::base::String, float>;
template class std::unordered_map<float, sf::base::String>;
template class std::unordered_map<sf::base::String, sf::base::String>;

template class std::unordered_set<sf::base::String>;

template class std::function<void()>;
template class std::function<bool()>;
template class std::function<sf::base::String()>;
