// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include "SSVOpenHexagon/Input/Combo.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Menu.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg
{

class BindControlBase : public ssvms::ItemBase
{
protected:
    bool waitingForBind{false};
    int  ID;

public:
    explicit BindControlBase(ssvms::Menu& mMenu, ssvms::Category& mCategory, const sf::base::String& mName, const int mID) :
        ssvms::ItemBase(mMenu, mCategory, mName),
        ID{mID}
    {
    }

    [[nodiscard]] virtual bool erase()                  = 0;
    [[nodiscard]] virtual bool isWaitingForBind() const = 0;
};

class KeyboardBindControl final : public BindControlBase
{
private:
    using Trigger       = ssvs::Input::Trigger;
    using TriggerGetter = sf::base::FixedFunction<ssvs::Input::Trigger&(), 64>;
    using SizeGetter    = sf::base::FixedFunction<int(), 64>;
    using AddBind       = sf::base::FixedFunction<void(const sf::Keyboard::Key, const sf::Mouse::Button), 64>;
    using Callback      = sf::base::FixedFunction<void(const ssvs::Input::Trigger&, const int), 64>;

    TriggerGetter                       triggerGetter;
    SizeGetter                          sizeGetter;
    AddBind                             addBind;
    sf::base::FixedFunction<void(), 64> clearBind;
    Callback                            callback;
    // A few actions have hardcoded keys, user should not be allowed to
    // bind the hardcoded key a second time.
    sf::Keyboard::Key hardcodedKey;

    [[nodiscard]] int getRealSize(const sf::base::Vector<ssvs::Input::Combo>& combos) const;
    void              applyBind(const sf::Keyboard::Key key, const sf::Mouse::Button);

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncClear, typename TFuncCallback>
    explicit KeyboardBindControl(
        ssvms::Menu&            mMenu,
        ssvms::Category&        mCategory,
        const sf::base::String& mName,
        TFuncGet                mFuncGet,
        TFuncSet                mFuncSet,
        TFuncClear              mFuncClear,
        TFuncCallback           mCallback,
        const int               mTriggerID,
        const sf::Keyboard::Key mHardcodedKey = sf::Keyboard::Key::Unknown) :
        BindControlBase{mMenu, mCategory, mName, mTriggerID},
        triggerGetter{mFuncGet},
        sizeGetter{[this] { return getRealSize(triggerGetter().getCombos()); }},
        addBind{[this, mFuncSet](const sf::Keyboard::Key setKey, const sf::Mouse::Button setBtn)
    { mFuncSet(setKey, setBtn, sizeGetter()); }},
        clearBind{[this, mFuncClear] { mFuncClear(sizeGetter() - 1); }},
        callback{mCallback},
        hardcodedKey{mHardcodedKey}
    {
        // If user manually added a hardcoded key to the config file
        // sanitize the bind. Cannot use a reference here because
        // `triggerGetter()` returns by value.
        const sf::base::Vector<ssvs::Input::Combo> combos{triggerGetter().getCombos()};

        for (int i = 0; i < static_cast<int>(combos.size()); ++i)
        {
            if (combos[i].getKeys()[int(hardcodedKey) + 1])
            {
                mFuncClear(i);
            }
        }
    }

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() const override;
    [[nodiscard]] bool erase() override;

    bool newKeyboardBind(const sf::Keyboard::Key key);
    bool newKeyboardBind(const sf::Mouse::Button btn);

    [[nodiscard]] sf::base::String getName() const override;
};

class JoystickBindControl final : public BindControlBase
{
private:
    using ValueGetter = sf::base::FixedFunction<unsigned int(), 64>;
    using ValueSetter = sf::base::FixedFunction<void(const unsigned int), 64>;
    using Callback    = sf::base::FixedFunction<void(const unsigned int, const int), 64>;

    ValueGetter valueGetter;
    ValueSetter setButton;
    Callback    callback;

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncCallback>
    explicit JoystickBindControl(ssvms::Menu&            mMenu,
                                 ssvms::Category&        mCategory,
                                 const sf::base::String& mName,
                                 TFuncGet                mFuncGet,
                                 TFuncSet                mFuncSet,
                                 TFuncCallback           mCallback,
                                 const int               mButtonID) :
        BindControlBase{mMenu, mCategory, mName, mButtonID},
        valueGetter{mFuncGet},
        setButton{mFuncSet},
        callback{mCallback}
    {
    }

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() const override;
    [[nodiscard]] bool erase() override;

    void newJoystickBind(const unsigned int joy);

    [[nodiscard]] sf::base::String getName() const override;
};

} // namespace hg
// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/BindControl.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"
#include "SSVOpenHexagon/Input/Combo.hpp"
#include "SSVOpenHexagon/Input/Manager.hpp"
#include "SSVOpenHexagon/Input/Trigger.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/ItemBase.hpp"
#include "SSVOpenHexagon/MenuSystem/Menu/Menu.hpp"

#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"

#include "SFML/Base/String.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg
{

[[nodiscard]] int KeyboardBindControl::getRealSize(const sf::base::Vector<ssvs::Input::Combo>& combos) const
{
    decltype(combos.size()) i = 0;
    for (; i < combos.size(); ++i)
    {
        if (combos[i].isUnbound())
        {
            break;
        }
    }

    return i;
}

void KeyboardBindControl::exec()
{
    waitingForBind = !waitingForBind;
}

[[nodiscard]] bool KeyboardBindControl::isWaitingForBind() const
{
    return waitingForBind;
}

[[nodiscard]] bool KeyboardBindControl::erase()
{
    const int size = sizeGetter();
    if (!size)
    {
        return false;
    }

    clearBind();
    callback(triggerGetter(), ID);
    return true;
}

bool KeyboardBindControl::newKeyboardBind(const sf::Keyboard::Key key)
{
    if (key == hardcodedKey)
    {
        waitingForBind = false;
        return false;
    }

    // stop if the pressed key is already assigned to this bind
    const sf::base::Vector<ssvs::Input::Combo> combos = triggerGetter().getCombos();

    for (int i = 0; i < sizeGetter(); ++i)
    {
        if (combos[i].getKeys()[int(key) + 1])
        {
            waitingForBind = false;
            return true;
        }
    }

    applyBind(key, sf::Mouse::Button::Left);
    return true;
}

bool KeyboardBindControl::newKeyboardBind(const sf::Mouse::Button btn)
{
    // stop if the pressed key is already assigned to this bind
    const sf::base::Vector<ssvs::Input::Combo> combos = triggerGetter().getCombos();

    for (int i = 0; i < sizeGetter(); ++i)
    {
        if (combos[i].getBtns()[int(btn) + 1])
        {
            waitingForBind = false;
            return true;
        }
    }

    applyBind(sf::Keyboard::Key::Unknown, btn);
    return true;
}

void KeyboardBindControl::applyBind(const sf::Keyboard::Key key, const sf::Mouse::Button btn)
{
    // assign the pressed key to the config value
    addBind(key, btn);

    // apply the new bind in game
    callback(triggerGetter(), ID);

    // finalize
    waitingForBind = false;
}

[[nodiscard]] sf::base::String KeyboardBindControl::getName() const
{
    sf::base::String bindNames = Config::getKeyboardBindNames(static_cast<Config::Tid>(ID));

    if (waitingForBind)
    {
        bindNames += "_";
    }

    return name + ": " + bindNames;
}

void JoystickBindControl::exec()
{
    waitingForBind = !waitingForBind;
}

[[nodiscard]] bool JoystickBindControl::isWaitingForBind() const
{
    return waitingForBind;
}

[[nodiscard]] bool JoystickBindControl::erase()
{
    if (valueGetter() == 33)
    {
        return false;
    }

    // clear both the config and the in game input
    setButton(33);
    callback(33, ID);
    return true;
}

void JoystickBindControl::newJoystickBind(const unsigned int joy)
{
    // stop if the pressed button is already assigned to this bind
    if (joy == valueGetter())
    {
        waitingForBind = false;
        return;
    }

    // save the new key in config
    setButton(joy);

    // update the bind we customized
    callback(joy, ID);

    // finalize
    waitingForBind = false;
}

[[nodiscard]] sf::base::String JoystickBindControl::getName() const
{
    sf::base::String bindName = Config::getJoystickBindName(static_cast<Joystick::Jid>(ID));

    if (waitingForBind)
    {
        bindName += "_";
    }

    return name + ": " + bindName;
}

} // namespace hg
