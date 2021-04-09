// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

#include <SSVMenuSystem/Global/Typedefs.hpp>
#include <SSVMenuSystem/Menu/ItemBase.hpp>
#include <SSVMenuSystem/Menu/Menu.hpp>

#include <SSVStart/Input/Input.hpp>

#include <string>

namespace hg {

class BindControlBase : public ssvms::ItemBase
{
protected:
    bool waitingForBind{false};
    int ID;

public:
    explicit BindControlBase(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, const int mID)
        : ssvms::ItemBase(mMenu, mCategory, mName), ID{mID}
    {}

    [[nodiscard]] virtual bool erase() = 0;
    [[nodiscard]] virtual bool isWaitingForBind() const = 0;
};

class KeyboardBindControl final : public BindControlBase
{
private:
    using Trigger = ssvs::Input::Trigger;
    using TriggerGetter = std::function<ssvs::Input::Trigger()>;
    using SizeGetter = std::function<int()>;
    using AddBind = std::function<void(const ssvs::KKey, const ssvs::MBtn)>;
    using Callback =
        std::function<void(const ssvs::Input::Trigger&, const int)>;

    TriggerGetter triggerGetter;
    SizeGetter sizeGetter;
    AddBind addBind;
    ssvms::Action clearBind;
    Callback callback;
    // A few actions have hardcoded keys, user should not be allowed to
    // bind the hardcoded key a second time.
    ssvs::KKey hardcodedKey;

    [[nodiscard]] int getRealSize(
        const std::vector<ssvs::Input::Combo>& combos) const;
    void applyBind(const ssvs::KKey key, const ssvs::MBtn);

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncClear,
        typename TFuncCallback>
    explicit KeyboardBindControl(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet,
        TFuncClear mFuncClear, TFuncCallback mCallback, const int mTriggerID,
        const ssvs::KKey mHardcodedKey = ssvs::KKey::Unknown)
        : BindControlBase{mMenu, mCategory, mName, mTriggerID},
          triggerGetter{mFuncGet},
          sizeGetter{
              [this] { return getRealSize(triggerGetter().getCombos()); }},
          addBind{
              [this, mFuncSet](const ssvs::KKey setKey, const ssvs::MBtn setBtn)
              { mFuncSet(setKey, setBtn, sizeGetter()); }},
          clearBind{[this, mFuncClear] { mFuncClear(sizeGetter() - 1); }},
          callback{mCallback},
          hardcodedKey{mHardcodedKey}
    {
        // If user manually added a hardcoded key to the config file
        // sanitize the bind. Cannot use a reference here because
        // `triggerGetter()` returns by value.
        const std::vector<ssvs::Input::Combo> combos{
            triggerGetter().getCombos()};

        for(int i = 0; i < static_cast<int>(combos.size()); ++i)
        {
            if(combos.at(i).getKeys()[int(hardcodedKey) + 1])
            {
                mFuncClear(i);
            }
        }
    }

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() const override;
    [[nodiscard]] bool erase() override;

    bool newKeyboardBind(const ssvs::KKey key);
    bool newKeyboardBind(const ssvs::MBtn btn);

    [[nodiscard]] std::string getName() const override;
};

class JoystickBindControl final : public BindControlBase
{
private:
    using ValueGetter = std::function<unsigned int()>;
    using ValueSetter = std::function<void(const unsigned int)>;
    using Callback = std::function<void(const unsigned int, const int)>;

    ValueGetter valueGetter;
    ValueSetter setButton;
    Callback callback;

public:
    template <typename TFuncGet, typename TFuncSet, typename TFuncCallback>
    explicit JoystickBindControl(ssvms::Menu& mMenu, ssvms::Category& mCategory,
        const std::string& mName, TFuncGet mFuncGet, TFuncSet mFuncSet,
        TFuncCallback mCallback, const int mButtonID)
        : BindControlBase{mMenu, mCategory, mName, mButtonID},
          valueGetter{mFuncGet},
          setButton{mFuncSet},
          callback{mCallback}
    {}

    void exec() override;

    [[nodiscard]] bool isWaitingForBind() const override;
    [[nodiscard]] bool erase() override;

    void newJoystickBind(const unsigned int joy);

    [[nodiscard]] std::string getName() const override;
};

} // namespace hg
