// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVMS_ITEM_BIND
#define SSVMS_ITEM_BIND

#include <extlibs/SSVMenuSystem/include/SSVMenuSystem/Global/Typedefs.hpp>
#include <extlibs/SSVMenuSystem/include/SSVMenuSystem/Menu/ItemBase.hpp>
#include <extlibs/SSVMenuSystem/include/SSVMenuSystem/Menu/Menu.hpp>
#include <extlibs/SSVStart/include/SSVStart/Input/Input.hpp>
#include <string>

#define MS_VENDOR_ID 0x045E
#define SONY_VENDOR_ID 0x54C

namespace ssvms
{
	class Menu;
	class Category;

	namespace Items
	{
        enum BindType
        {
            KeyboardBind = 1,
            JoystickBind
        };

        class BindControlBase : public ItemBase
        {
        protected:
            bool waitingForBind{false};
            int ID;

        public:

            BindControlBase(Menu& mMenu, Category& mCategory, const std::string& mName, const int mID)
            : ItemBase(mMenu, mCategory, mName), ID{mID}
            {
            }

            inline virtual bool erase() { return false; }
            inline virtual void newKeyboardBind(const KKey key = KKey::Unknown, const MBtn btn = MBtn::Left) { (void)(key); (void)(btn); }
            inline virtual void newJoystickBind(const int joy = -1) { (void)(joy); }
            inline virtual int isWaitingForBind() { return 0; }
        };

        class BindControl final : public BindControlBase
        {
        private:
            using Combo = ssvs::Input::Combo;
            using KKey = ssvs::KKey;
            using MBtn = ssvs::MBtn;
            using Trigger = ssvs::Input::Trigger;
            using TriggerGetter = std::function<Trigger()>;
            using SizeGetter = std::function<int()>;
            using BindReturn = std::function<std::pair<int, Trigger>(KKey, MBtn)>;
            using Callback = std::function<void(const Trigger&, const int)>;

            TriggerGetter triggerGetter;
            SizeGetter sizeGetter;
            BindReturn addBind;
            Action clearBind;
            Callback callback;

            [[nodiscard]] inline int getRealSize(const std::vector<Combo>& combos) const
            {
                int i, size = combos.size();
                for (i = 0; i < size; ++i)
				{
                    if(combos[i].isUnbound())
						break;
				}
                return i;
            }

        public:
            template <typename TFuncGet, typename TFuncSet, typename TFuncClear, typename TFuncCallback>
            BindControl(Menu& mMenu, Category& mCategory, const std::string& mName,
                        TFuncGet mFuncGet, TFuncSet mFuncSet, TFuncClear mFuncClear,
                        TFuncCallback mCallback, int mTriggerID)
                : BindControlBase{mMenu, mCategory, mName, mTriggerID},
                  triggerGetter{[=, this] { return mFuncGet(); }},
                  sizeGetter{[=, this] { return getRealSize(triggerGetter().getCombos()); }},
                  addBind{[=, this](const KKey setKey, const MBtn setBtn) { return mFuncSet(setKey, setBtn, sizeGetter()); }},
                  clearBind{[=, this] { mFuncClear(sizeGetter()); }},
                  callback{mCallback}
            {
            }

            inline void exec() override { waitingForBind = !waitingForBind; }
            inline int isWaitingForBind() override { return waitingForBind ? KeyboardBind : 0; }
            inline bool erase() override
            {
                const int size = sizeGetter();
                if(!size) { return false; }

                clearBind();
                callback(triggerGetter(), ID);
                return true;
            }

            inline void newKeyboardBind(const KKey key, const MBtn btn) override
            {
                // stop if the pressed key is already assigned to this bind
                const std::vector<Combo>& Combos = triggerGetter().getCombos();
                int size = sizeGetter();
                if(key > KKey::Unknown)
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
                if(unboundID > -1) { callback(trig, unboundID); }

                // apply the new bind in game
                callback(triggerGetter(), ID);

                // finalize
                waitingForBind = false;
            }

            inline std::string getName() const override
            {
                const std::vector<Combo>& combos = triggerGetter().getCombos();
                const int size = sizeGetter();
                std::string bindNames;
                std::bitset<102> keyBind;
                std::bitset<6> btnBinds;
                int i, j;

                //get binds in the order they have been entered
                for(i = 0; i < size; ++i)
                {
                    keyBind = combos[i].getKeys();
                    for(j = 0; j <= KKey::KeyCount; ++j)
                    {
                        if(keyBind[j])
                        {
                            if(!bindNames.empty()) { bindNames += ", "; }
                            bindNames += ssvs::getKKeyName(KKey(j - 1)); //names are shifted compared to the Key enum
                            break;
                        }
                    }

                    btnBinds = combos[i].getBtns();
                    for(j = 0; j <= MBtn::ButtonCount; ++j)
                    {
                        if(btnBinds[j])
                        {
                            if(!bindNames.empty()) { bindNames += ", "; }
                            bindNames += ssvs::getMBtnName(MBtn(j - 1)); //same as with keys
                            break;
                        }
                    }
                }

                if(waitingForBind) { bindNames += "_"; }

                return name + ": " + bindNames;
            }
        };

        class JoystickBindControl final : public BindControlBase
        {
        private:
            using ValueGetter = std::function<int()>;
            using ValueSetter = std::function<int(const int)>;
            using Callback = std::function<void(const unsigned int , const int)>;

            ValueGetter valueGetter;
            ValueSetter setButton;
            Callback callback;

            const std::string buttonsNames[12][2] = {
                { "A", "SQUARE" }, 			{ "B", "CROSS" }, 				{ "X", "CIRCLE" }, 		{ "Y", "TRIANGLE" },
                { "LB", "L1" }, 			{ "RB", "R1" }, 				{ "BACK", "L2" }, 		{ "START", "R2" },
                { "LEFT STICK", "SELECT" }, { "RIGHT STICK", "START" }, 	{ "LT", "LEFT STICK" }, { "RT", "RIGHT STICK" } };

        public:
            template <typename TFuncGet, typename TFuncSet, typename TFuncCallback>
            JoystickBindControl(Menu& mMenu, Category& mCategory, const std::string& mName,
                                TFuncGet mFuncGet, TFuncSet mFuncSet, TFuncCallback mCallback, int mButtonID)
                                : BindControlBase{mMenu, mCategory, mName, mButtonID},
                                valueGetter{[=] { return mFuncGet(); }},
                                setButton{[=, this](const int pressedButton) { return mFuncSet(pressedButton); }},
                                callback{mCallback}
            {
            }

            inline void exec() override { waitingForBind = !waitingForBind; }
            inline int isWaitingForBind() override { return waitingForBind ? JoystickBind : 0; }
            inline bool erase() override
            {
                if(valueGetter() == 33) { return false; }

                // clear both the config and the in game input
                setButton(33);
                callback(33, ID);
                return true;
            }

            inline void newJoystickBind(const int joy) override
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
                if(unboundID > -1) { callback(33, unboundID); }

                // update the bind we customized
                callback(joy, ID);

                // finalize
                waitingForBind = false;
            }

            inline std::string getName() const override
            {
                std::string bindNames;
                unsigned int value = valueGetter();

                if(value == 33)
                {
                    bindNames = "";
                }
                else
                {
                    unsigned int vendorId = sf::Joystick::isConnected(0) ? sf::Joystick::getIdentification(0).vendorId : 0;
                    switch(vendorId)
                    {
                        case MS_VENDOR_ID:
                            bindNames = value >= 12 ? "" : buttonsNames[value][0];
                            break;
                        case SONY_VENDOR_ID:
                            bindNames = value >= 12 ? "" : buttonsNames[value][1];
                            break;
                        default:
                            bindNames = ssvu::toStr(value);
                            break;
                    }
                }

                if(waitingForBind) { bindNames += "_"; }

                return name + ": " + bindNames;
            }
        };
	}
}

#endif
