// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Core/BindControl.hpp"

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

[[nodiscard]] bool BindControlBase::isWaitingForBind()
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

[[nodiscard]] bool KeyboardBindControl::isWaitingForBind()
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

void KeyboardBindControl::newKeyboardBind(
    const ssvs::KKey key, const ssvs::MBtn btn)
{
    // stop if the pressed key is already assigned to this bind
    const std::vector<ssvs::Input::Combo>& Combos = triggerGetter().getCombos();

    const int size = sizeGetter();
    if(key > ssvs::KKey::Unknown)
    {
        for(int i = 0; i < size; ++i)
        {
            if(Combos[i].getKeys()[int(key) + 1])
            {
                waitingForBind = false;
                return;
            }
        }
    }
    else
    {
        for(int i = 0; i < size; ++i)
        {
            if(Combos[i].getBtns()[int(btn) + 1])
            {
                waitingForBind = false;
                return;
            }
        }
    }

    // assign the pressed key to the config value
    addBind(key, btn);

    // apply the new bind in game
    callback(triggerGetter(), ID);

    // finalize
    waitingForBind = false;
}

[[nodiscard]] std::string KeyboardBindControl::getName() const
{
    const std::vector<ssvs::Input::Combo>& combos = triggerGetter().getCombos();

    const int size = sizeGetter();
    std::string bindNames;

    // get binds in the order they have been entered
    for(int i = 0; i < size; ++i)
    {
        const auto keyBind = combos[i].getKeys();
        for(int j = 0; j <= ssvs::KKey::KeyCount; ++j)
        {
            if(!keyBind[j])
            {
                continue;
            }

            if(!bindNames.empty())
            {
                bindNames += ", ";
            }

            // names are shifted compared to the Key enum
            bindNames +=
                bindToHumanReadableName(ssvs::getKKeyName(ssvs::KKey(j - 1)));
            break;
        }

        const auto btnBinds = combos[i].getBtns();
        for(int j = 0; j <= ssvs::MBtn::ButtonCount; ++j)
        {
            if(!btnBinds[j])
            {
                continue;
            }

            if(!bindNames.empty())
            {
                bindNames += ", ";
            }

            // same as with keys
            bindNames +=
                bindToHumanReadableName(ssvs::getMBtnName(ssvs::MBtn(j - 1)));
            break;
        }
    }

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

[[nodiscard]] bool JoystickBindControl::isWaitingForBind()
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
    static const std::string buttonsNames[12][2] = {{"A", "SQUARE"},
        {"B", "CROSS"}, {"X", "CIRCLE"}, {"Y", "TRIANGLE"}, {"LB", "L1"},
        {"RB", "R1"}, {"BACK", "L2"}, {"START", "R2"}, {"LEFT STICK", "SELECT"},
        {"RIGHT STICK", "START"}, {"LT", "LEFT STICK"}, {"RT", "RIGHT STICK"}};

    std::string bindNames;
    const unsigned int value = valueGetter();

    if(value == 33)
    {
        bindNames = "";
    }
    else
    {
#define MS_VENDOR_ID 0x045E
#define SONY_VENDOR_ID 0x54C

        const unsigned int vendorId =
            sf::Joystick::isConnected(0)
                ? sf::Joystick::getIdentification(0).vendorId
                : 0;

        switch(vendorId)
        {
            case MS_VENDOR_ID:
                bindNames = value >= 12 ? "" : buttonsNames[value][0];
                break;
            case SONY_VENDOR_ID:
                bindNames = value >= 12 ? "" : buttonsNames[value][1];
                break;
            default: bindNames = ssvu::toStr(value); break;
        }

#undef MS_VENDOR_ID
#undef SONY_VENDOR_ID
    }

    if(waitingForBind)
    {
        bindNames += "_";
    }

    return name + ": " + bindNames;
}

} // namespace hg
