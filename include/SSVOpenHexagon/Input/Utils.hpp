// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/GameSystem/GameState.hpp"
#include "SSVOpenHexagon/Input/Enums.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include <map>
#include <string>

#include <cassert>

namespace ssvs
{

using ITrigger = Input::Trigger;
using IType    = Input::Type;
using IMode    = Input::Mode;

#define SSVS_KEY_PREFIX "k"
#define SSVS_BTN_PREFIX "b"

namespace Impl
{

inline const auto* getKKeyStrArray() noexcept
{
#define SSVS_INS_KEY(name) SSVS_KEY_PREFIX #name
    static std::string
        keys[]{SSVS_INS_KEY(A),         SSVS_INS_KEY(B),         SSVS_INS_KEY(C),        SSVS_INS_KEY(D),
               SSVS_INS_KEY(E),         SSVS_INS_KEY(F),         SSVS_INS_KEY(G),        SSVS_INS_KEY(H),
               SSVS_INS_KEY(I),         SSVS_INS_KEY(J),         SSVS_INS_KEY(K),        SSVS_INS_KEY(L),
               SSVS_INS_KEY(M),         SSVS_INS_KEY(N),         SSVS_INS_KEY(O),        SSVS_INS_KEY(P),
               SSVS_INS_KEY(Q),         SSVS_INS_KEY(R),         SSVS_INS_KEY(S),        SSVS_INS_KEY(T),
               SSVS_INS_KEY(U),         SSVS_INS_KEY(V),         SSVS_INS_KEY(W),        SSVS_INS_KEY(X),
               SSVS_INS_KEY(Y),         SSVS_INS_KEY(Z),         SSVS_INS_KEY(Num0),     SSVS_INS_KEY(Num1),
               SSVS_INS_KEY(Num2),      SSVS_INS_KEY(Num3),      SSVS_INS_KEY(Num4),     SSVS_INS_KEY(Num5),
               SSVS_INS_KEY(Num6),      SSVS_INS_KEY(Num7),      SSVS_INS_KEY(Num8),     SSVS_INS_KEY(Num9),
               SSVS_INS_KEY(Escape),    SSVS_INS_KEY(LControl),  SSVS_INS_KEY(LShift),   SSVS_INS_KEY(LAlt),
               SSVS_INS_KEY(LSystem),   SSVS_INS_KEY(RControl),  SSVS_INS_KEY(RShift),   SSVS_INS_KEY(RAlt),
               SSVS_INS_KEY(RSystem),   SSVS_INS_KEY(Menu),      SSVS_INS_KEY(LBracket), SSVS_INS_KEY(RBracket),
               SSVS_INS_KEY(Semicolon), SSVS_INS_KEY(Comma),     SSVS_INS_KEY(Period),   SSVS_INS_KEY(Apostrophe),
               SSVS_INS_KEY(Slash),     SSVS_INS_KEY(Backslash), SSVS_INS_KEY(Grave),    SSVS_INS_KEY(Equal),
               SSVS_INS_KEY(Hyphen),    SSVS_INS_KEY(Space),     SSVS_INS_KEY(Enter),    SSVS_INS_KEY(Backspace),
               SSVS_INS_KEY(Tab),       SSVS_INS_KEY(PageUp),    SSVS_INS_KEY(PageDown), SSVS_INS_KEY(End),
               SSVS_INS_KEY(Home),      SSVS_INS_KEY(Insert),    SSVS_INS_KEY(Delete),   SSVS_INS_KEY(Add),
               SSVS_INS_KEY(Subtract),  SSVS_INS_KEY(Multiply),  SSVS_INS_KEY(Divide),   SSVS_INS_KEY(Left),
               SSVS_INS_KEY(Right),     SSVS_INS_KEY(Up),        SSVS_INS_KEY(Down),     SSVS_INS_KEY(Numpad0),
               SSVS_INS_KEY(Numpad1),   SSVS_INS_KEY(Numpad2),   SSVS_INS_KEY(Numpad3),  SSVS_INS_KEY(Numpad4),
               SSVS_INS_KEY(Numpad5),   SSVS_INS_KEY(Numpad6),   SSVS_INS_KEY(Numpad7),  SSVS_INS_KEY(Numpad8),
               SSVS_INS_KEY(Numpad9),   SSVS_INS_KEY(F1),        SSVS_INS_KEY(F2),       SSVS_INS_KEY(F3),
               SSVS_INS_KEY(F4),        SSVS_INS_KEY(F5),        SSVS_INS_KEY(F6),       SSVS_INS_KEY(F7),
               SSVS_INS_KEY(F8),        SSVS_INS_KEY(F9),        SSVS_INS_KEY(F10),      SSVS_INS_KEY(F11),
               SSVS_INS_KEY(F12),       SSVS_INS_KEY(F13),       SSVS_INS_KEY(F14),      SSVS_INS_KEY(F15),
               SSVS_INS_KEY(Pause)};
#undef SSVS_INS_KEY

    return keys;
}

inline const auto* getMBtnStrArray() noexcept
{
#define SSVS_INS_BTN(name) SSVS_BTN_PREFIX #name
    static std::string buttons[]{SSVS_INS_BTN(Left),
                                 SSVS_INS_BTN(Right),
                                 SSVS_INS_BTN(Middle),
                                 SSVS_INS_BTN(Extra1),
                                 SSVS_INS_BTN(Extra2)};
#undef SSVS_INS_BTN

    return buttons;
}

inline const auto& getStrKKeyMap() noexcept
{
#define SSVS_INS_KEY(name)                             \
    {                                                  \
        SSVS_KEY_PREFIX #name, sf::Keyboard::Key::name \
    }
    static std::map<std::string, sf::Keyboard::Key>
        keys{SSVS_INS_KEY(A),         SSVS_INS_KEY(B),         SSVS_INS_KEY(C),        SSVS_INS_KEY(D),
             SSVS_INS_KEY(E),         SSVS_INS_KEY(F),         SSVS_INS_KEY(G),        SSVS_INS_KEY(H),
             SSVS_INS_KEY(I),         SSVS_INS_KEY(J),         SSVS_INS_KEY(K),        SSVS_INS_KEY(L),
             SSVS_INS_KEY(M),         SSVS_INS_KEY(N),         SSVS_INS_KEY(O),        SSVS_INS_KEY(P),
             SSVS_INS_KEY(Q),         SSVS_INS_KEY(R),         SSVS_INS_KEY(S),        SSVS_INS_KEY(T),
             SSVS_INS_KEY(U),         SSVS_INS_KEY(V),         SSVS_INS_KEY(W),        SSVS_INS_KEY(X),
             SSVS_INS_KEY(Y),         SSVS_INS_KEY(Z),         SSVS_INS_KEY(Num0),     SSVS_INS_KEY(Num1),
             SSVS_INS_KEY(Num2),      SSVS_INS_KEY(Num3),      SSVS_INS_KEY(Num4),     SSVS_INS_KEY(Num5),
             SSVS_INS_KEY(Num6),      SSVS_INS_KEY(Num7),      SSVS_INS_KEY(Num8),     SSVS_INS_KEY(Num9),
             SSVS_INS_KEY(Escape),    SSVS_INS_KEY(LControl),  SSVS_INS_KEY(LShift),   SSVS_INS_KEY(LAlt),
             SSVS_INS_KEY(LSystem),   SSVS_INS_KEY(RControl),  SSVS_INS_KEY(RShift),   SSVS_INS_KEY(RAlt),
             SSVS_INS_KEY(RSystem),   SSVS_INS_KEY(Menu),      SSVS_INS_KEY(LBracket), SSVS_INS_KEY(RBracket),
             SSVS_INS_KEY(Semicolon), SSVS_INS_KEY(Comma),     SSVS_INS_KEY(Period),   SSVS_INS_KEY(Apostrophe),
             SSVS_INS_KEY(Slash),     SSVS_INS_KEY(Backslash), SSVS_INS_KEY(Grave),    SSVS_INS_KEY(Equal),
             SSVS_INS_KEY(Hyphen),    SSVS_INS_KEY(Space),     SSVS_INS_KEY(Enter),    SSVS_INS_KEY(Backspace),
             SSVS_INS_KEY(Tab),       SSVS_INS_KEY(PageUp),    SSVS_INS_KEY(PageDown), SSVS_INS_KEY(End),
             SSVS_INS_KEY(Home),      SSVS_INS_KEY(Insert),    SSVS_INS_KEY(Delete),   SSVS_INS_KEY(Add),
             SSVS_INS_KEY(Subtract),  SSVS_INS_KEY(Multiply),  SSVS_INS_KEY(Divide),   SSVS_INS_KEY(Left),
             SSVS_INS_KEY(Right),     SSVS_INS_KEY(Up),        SSVS_INS_KEY(Down),     SSVS_INS_KEY(Numpad0),
             SSVS_INS_KEY(Numpad1),   SSVS_INS_KEY(Numpad2),   SSVS_INS_KEY(Numpad3),  SSVS_INS_KEY(Numpad4),
             SSVS_INS_KEY(Numpad5),   SSVS_INS_KEY(Numpad6),   SSVS_INS_KEY(Numpad7),  SSVS_INS_KEY(Numpad8),
             SSVS_INS_KEY(Numpad9),   SSVS_INS_KEY(F1),        SSVS_INS_KEY(F2),       SSVS_INS_KEY(F3),
             SSVS_INS_KEY(F4),        SSVS_INS_KEY(F5),        SSVS_INS_KEY(F6),       SSVS_INS_KEY(F7),
             SSVS_INS_KEY(F8),        SSVS_INS_KEY(F9),        SSVS_INS_KEY(F10),      SSVS_INS_KEY(F11),
             SSVS_INS_KEY(F12),       SSVS_INS_KEY(F13),       SSVS_INS_KEY(F14),      SSVS_INS_KEY(F15),
             SSVS_INS_KEY(Pause)};
#undef SSVS_INS_KEY

    return keys;
}

inline const auto& getStrKKeyHardcodedMap() noexcept
{
#define SSVS_INS_KEY(name)                             \
    {                                                  \
        SSVS_KEY_PREFIX #name, sf::Keyboard::Key::name \
    }
    static std::map<std::string, sf::Keyboard::Key>
        keys{SSVS_INS_KEY(J),
             SSVS_INS_KEY(K),
             SSVS_INS_KEY(L),
             SSVS_INS_KEY(R),
             SSVS_INS_KEY(Y),
             SSVS_INS_KEY(Escape),
             SSVS_INS_KEY(LAlt),
             SSVS_INS_KEY(Enter),
             SSVS_INS_KEY(Backspace),
             SSVS_INS_KEY(Up),
             SSVS_INS_KEY(Down),
             SSVS_INS_KEY(F1),
             SSVS_INS_KEY(F2),
             SSVS_INS_KEY(F3),
             SSVS_INS_KEY(F4)};
#undef SSVS_INS_KEY

    return keys;
}

inline const auto& getStrMBtnMap() noexcept
{
#define SSVS_INS_BTN(name)                             \
    {                                                  \
        SSVS_BTN_PREFIX #name, sf::Mouse::Button::name \
    }
    static std::map<std::string, sf::Mouse::Button>
        buttons{SSVS_INS_BTN(Left), SSVS_INS_BTN(Right), SSVS_INS_BTN(Middle), SSVS_INS_BTN(Extra1), SSVS_INS_BTN(Extra2)};
#undef SSVS_INS_BTN

    return buttons;
}

} // namespace Impl

#undef SSVS_KEY_PREFIX
#undef SSVS_BTN_PREFIX

[[nodiscard]] inline bool isKKeyNameValid(const std::string& id) noexcept
{
    return Impl::getStrKKeyMap().contains(id);
}

[[nodiscard]] inline bool isKKeyHardcoded(const std::string& id) noexcept
{
    return Impl::getStrKKeyHardcodedMap().contains(id);
}

[[nodiscard]] inline bool isMBtnNameValid(const std::string& id) noexcept
{
    return Impl::getStrMBtnMap().contains(id);
}

[[nodiscard]] inline sf::Keyboard::Key getKKey(const std::string& id) noexcept
{
    assert(isKKeyNameValid(id));
    return Impl::getStrKKeyMap().at(id);
}

[[nodiscard]] inline sf::Mouse::Button getMBtn(const std::string& id) noexcept
{
    assert(isMBtnNameValid(id));
    return Impl::getStrMBtnMap().at(id);
}

[[nodiscard]] inline const auto& getKKeyName(const sf::Keyboard::Key key) noexcept
{
    static const std::string unknown;
    const auto               idx = static_cast<int>(key);
    if (idx < 0 || idx > static_cast<int>(sf::Keyboard::Key::Pause))
        return unknown;
    return Impl::getKKeyStrArray()[idx];
}

[[nodiscard]] inline const auto& getMBtnName(const sf::Mouse::Button button) noexcept
{
    return Impl::getMBtnStrArray()[static_cast<int>(button)];
}

inline auto& add2StateInput(GameState&      gameState,
                            const ITrigger& on,
                            bool&           value,
                            const int       triggerID,
                            const IType     type = IType::Always,
                            const IMode     mode = IMode::Overlap)
{
    return gameState.addInput(on, [&value](float) { value = true; }, [&value](float) {
        value = false;
    }, type, triggerID, mode);
}

} // namespace ssvs
