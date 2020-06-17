// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#pragma once

namespace ssvuj
{
namespace Impl
{
class LinkedValueBase
{
protected:
    std::string name;

public:
    LinkedValueBase(std::string mLinkedName) : name{ssvu::mv(mLinkedName)}
    {
    }
    virtual ~LinkedValueBase()
    {
    }

    virtual void syncFrom(const Obj& mRoot) = 0;
    virtual void syncTo(Obj& mRoot) const = 0;
};
} // namespace Impl

template <typename T>
class LinkedValue final : public Impl::LinkedValueBase
{
private:
    T value;

public:
    LinkedValue(std::string mLinkedName)
        : Impl::LinkedValueBase{ssvu::mv(mLinkedName)}
    {
    }

    operator T() const noexcept
    {
        return value;
    }
    auto& operator=(const T& mValue)
    {
        value = mValue;
        return *this;
    }

    void syncFrom(const Obj& mObj) override
    {
        extr(mObj, name, value);
    }
    void syncTo(Obj& mObj) const override
    {
        arch(mObj, name, value);
    }
};

class LinkedValueManager
{
private:
    using Container = ssvu::VecUPtr<Impl::LinkedValueBase>;
    Obj& obj;
    Container values;

public:
    LinkedValueManager(Obj& mObj) : obj(mObj)
    {
    }

    template <typename T>
    auto& create(std::string mName)
    {
        return ssvu::getEmplaceUPtr<LinkedValue<T>>(values, ssvu::mv(mName));
    }

    void syncFromObj()
    {
        for(auto& lv : values) lv->syncFrom(obj);
    }
    void syncToObj() const
    {
        for(const auto& lv : values) lv->syncTo(obj);
    }

    const auto& getValues() const noexcept
    {
        return values;
    }
};

} // namespace ssvuj
