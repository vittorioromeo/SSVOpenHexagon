// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/BindControl.hpp"
#include "SSVOpenHexagon/Global/Config.hpp"

#include <SSVMenuSystem/Global/Typedefs.hpp>
#include <SSVMenuSystem/Menu/ItemBase.hpp>
#include <SSVMenuSystem/Menu/Menu.hpp>

#include <SSVStart/Input/Input.hpp>
#include <SSVStart/Utils/Input.hpp>

#include <string>

namespace hg
{

BindControlBase::BindControlBase(ssvms::Menu& mMenu, ssvms::Category& mCategory,
    const std::string& mName, const int mID)
    : ssvms::ItemBase(mMenu, mCategory, mName), ID{mID}
{
}

[[nodiscard]] bool BindControlBase::erase()
{
    return false;
}

[[nodiscard]] bool BindControlBase::isWaitingForBind() const
{
    return false;
}

// ---

[[nodiscard]] int KeyboardBindControl::getRealSize(
    const std::vector<ssvs::Input::Combo>& combos) const
{
    decltype(combos.size()) i = 0;
    for(; i < combos.size(); ++i)
    {
        if(combos[i].isUnbound())
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
    if(!size)
    {
        return false;
    }

    clearBind();
    callback(triggerGetter(), ID);
    return true;
}

bool KeyboardBindControl::newKeyboardBind(const ssvs::KKey key)
{
    if(key == hardcodedKey)
    {
        waitingForBind = false;
        return false;
    }

    // stop if the pressed key is already assigned to this bind
    const std::vector<ssvs::Input::Combo>& combos = triggerGetter().getCombos();

    for(int i = 0; i < sizeGetter(); ++i)
    {
        if(combos[i].getKeys()[int(key) + 1])
        {
            waitingForBind = false;
            return true;
        }
    }

    applyBind(key, ssvs::MBtn::Left);
    return true;
}

bool KeyboardBindControl::newKeyboardBind(const ssvs::MBtn btn)
{
    // stop if the pressed key is already assigned to this bind
    const std::vector<ssvs::Input::Combo>& combos = triggerGetter().getCombos();

    for(int i = 0; i < sizeGetter(); ++i)
    {
        if(combos[i].getBtns()[int(btn) + 1])
        {
            waitingForBind = false;
            return true;
        }
    }

    applyBind(ssvs::KKey::Unknown, btn);
    return true;
}

void KeyboardBindControl::applyBind(const ssvs::KKey key, const ssvs::MBtn btn)
{
    // assign the pressed key to the config value
    addBind(key, btn);

    // apply the new bind in game
    callback(triggerGetter(), ID);

    // finalize
    waitingForBind = false;
}

[[nodiscard]] std::string KeyboardBindControl::getName() const
{
    std::string bindNames = Config::getKeyboardBindNames(ID);

    if(waitingForBind)
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
    if(valueGetter() == 33)
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
    if(joy == valueGetter())
    {
        waitingForBind = false;
        return;
    }

    // save the new key in config
    int unboundID = setButton(joy);

    // if the key was bound to another function and it was reassigned
    // make sure we also update the unbound joystick button
    if(unboundID > -1)
    {
        callback(33, unboundID);
    }

    // update the bind we customized
    callback(joy, ID);

    // finalize
    waitingForBind = false;
}

[[nodiscard]] std::string JoystickBindControl::getName() const
{
    std::string bindName = Config::getJoystickBindNames(ID);

    if(waitingForBind)
    {
        bindName += "_";
    }

    return name + ": " + bindName;
}

} // namespace hg
