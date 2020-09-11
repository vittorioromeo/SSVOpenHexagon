// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVMenuSystem/Global/Typedefs.hpp>
#include <SSVMenuSystem/Menu/ItemBase.hpp>
#include <SSVMenuSystem/Menu/Menu.hpp>

#include <SSVStart/Input/Input.hpp>

#include <string>


namespace hg
{

class BindControlBase : public ssvms::ItemBase
{
protected:
    bool waitingForBind{false};
    int ID;

public:
    BindControlBase(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, const int mID)
        : ssvms::ItemBase(mMenu, mCategory, mName), ID{mID}
    {
    }

    [[nodiscard]] virtual bool erase()
    {
        return false;
    }

    [[nodiscard]] virtual bool isWaitingForBind()
    {
        return false;
    }
};

class KeyboardBindControl final : public BindControlBase
{
private:
    using Trigger = ssvs::Input::Trigger;
    using TriggerGetter = std::function<ssvs::Input::Trigger()>;
    using SizeGetter = std::function<int()>;
    using BindReturn =
        std::function<std::pair<int, Trigger>(ssvs::KKey, ssvs::MBtn)>;
    using Callback =
        std::function<void(const ssvs::Input::Trigger&, const int)>;

    TriggerGetter triggerGetter;
    SizeGetter sizeGetter;
    BindReturn addBind;
    ssvms::Action clearBind;
    Callback callback;

    [[nodiscard]] int getRealSize(
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

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncClear,
        typename TFuncCallback>
    KeyboardBindControl(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet,
        TFuncClear mFuncClear, TFuncCallback mCallback, int mTriggerID)
        : BindControlBase{mMenu, mCategory, mName, mTriggerID},
          triggerGetter{mFuncGet}, sizeGetter{[this] {
              return getRealSize(triggerGetter().getCombos());
          }},
          addBind{[this, mFuncSet](
                      const ssvs::KKey setKey, const ssvs::MBtn setBtn) {
              return mFuncSet(setKey, setBtn, sizeGetter());
          }},
          clearBind{[this, mFuncClear] { mFuncClear(sizeGetter()); }},
          callback{mCallback}
    {
    }

    void exec() override
    {
        waitingForBind = !waitingForBind;
    }

    [[nodiscard]] bool isWaitingForBind() override
    {
        return waitingForBind;
    }

    [[nodiscard]] bool erase() override
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

    void newKeyboardBind(
        const ssvs::KKey key, const ssvs::MBtn btn = ssvs::MBtn::Left)
    {
        // stop if the pressed key is already assigned to this bind
        const std::vector<ssvs::Input::Combo>& Combos =
            triggerGetter().getCombos();

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
        auto [unboundID, trig] = addBind(key, btn);

        // key was assigned to another function and was unbound.
        // This trigger must be refreshed as well
        if(unboundID > -1)
        {
            callback(trig, unboundID);
        }

        // apply the new bind in game
        callback(triggerGetter(), ID);

        // finalize
        waitingForBind = false;
    }

    std::string getName() const override
    {
        const std::vector<ssvs::Input::Combo>& combos =
            triggerGetter().getCombos();

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
                bindNames += ssvs::getKKeyName(ssvs::KKey(j - 1));
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
                bindNames += ssvs::getMBtnName(ssvs::MBtn(j - 1));
                break;
            }
        }

        if(waitingForBind)
        {
            bindNames += "_";
        }

        return name + ": " + bindNames;
    }
};

class JoystickBindControl final : public BindControlBase
{
private:
    using ValueGetter = std::function<int()>;
    using ValueSetter = std::function<int(const int)>;
    using Callback = std::function<void(const unsigned int, const int)>;

    ValueGetter valueGetter;
    ValueSetter setButton;
    Callback callback;

    const std::string buttonsNames[12][2] = {{"A", "SQUARE"}, {"B", "CROSS"},
        {"X", "CIRCLE"}, {"Y", "TRIANGLE"}, {"LB", "L1"}, {"RB", "R1"},
        {"BACK", "L2"}, {"START", "R2"}, {"LEFT STICK", "SELECT"},
        {"RIGHT STICK", "START"}, {"LT", "LEFT STICK"}, {"RT", "RIGHT STICK"}};

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncCallback>
    JoystickBindControl(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet,
        TFuncCallback mCallback, int mButtonID)
        : BindControlBase{mMenu, mCategory, mName, mButtonID},
          valueGetter{mFuncGet}, setButton{mFuncSet}, callback{mCallback}
    {
    }

    void exec() override
    {
        waitingForBind = !waitingForBind;
    }

    [[nodiscard]] bool isWaitingForBind() override
    {
        return waitingForBind;
    }

    [[nodiscard]] bool erase() override
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

    void newJoystickBind(const int joy)
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

    [[nodiscard]] std::string getName() const override
    {
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
};

} // namespace hg
