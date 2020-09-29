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
        const std::string& mName, const int mID);

    [[nodiscard]] virtual bool erase();
    [[nodiscard]] virtual bool isWaitingForBind();
};

class KeyboardBindControl final : public BindControlBase
{
private:
    using Trigger = ssvs::Input::Trigger;
    using TriggerGetter = std::function<ssvs::Input::Trigger()>;
    using SizeGetter = std::function<int()>;
    using AddBind = std::function<void(ssvs::KKey, ssvs::MBtn)>;
    using Callback =
        std::function<void(const ssvs::Input::Trigger&, const int)>;

    TriggerGetter triggerGetter;
    SizeGetter sizeGetter;
    AddBind addBind;
    ssvms::Action clearBind;
    Callback callback;

    [[nodiscard]] int getRealSize(
        const std::vector<ssvs::Input::Combo>& combos) const;

    [[nodiscard]] std::string bindToHumanReadableName(std::string s) const
    {
        if(s.starts_with('k'))
        {
            return s.substr(1);
        }

        if(s == "bLeft")
        {
            return "LMB";
        }

        if(s == "bRight")
        {
            return "RMB";
        }

        if(s == "bMiddle")
        {
            return "MMB";
        }

        return s;
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
              mFuncSet(setKey, setBtn, sizeGetter());
          }},
          clearBind{[this, mFuncClear] { mFuncClear(sizeGetter()); }},
          callback{mCallback}
    {
    }

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() override;
    [[nodiscard]] bool erase() override;

    void newKeyboardBind(
        const ssvs::KKey key, const ssvs::MBtn btn = ssvs::MBtn::Left);

    [[nodiscard]] std::string getName() const override;
};

class JoystickBindControl final : public BindControlBase
{
private:
    using ValueGetter = std::function<unsigned int()>;
    using ValueSetter = std::function<int(const unsigned int)>;
    using Callback = std::function<void(const unsigned int, const unsigned int)>;

    ValueGetter valueGetter;
    ValueSetter setButton;
    Callback callback;

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncCallback>
    JoystickBindControl(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet,
        TFuncCallback mCallback, int mButtonID)
        : BindControlBase{mMenu, mCategory, mName, mButtonID},
          valueGetter{mFuncGet}, setButton{mFuncSet}, callback{mCallback}
    {
    }

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() override;
    [[nodiscard]] bool erase() override;

    void newJoystickBind(const unsigned int joy);

    [[nodiscard]] std::string getName() const override;
};

} // namespace hg
