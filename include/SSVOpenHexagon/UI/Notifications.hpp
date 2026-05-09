// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#pragma once

// Lightweight, immediate-mode toast notifications for the bottom-right
// of the screen. Used for state changes that aren't worth a modal --
// "connection success", "pack downloaded", etc.
//
// No dependencies on widgets/screens; the host (`MenuGame`) owns one
// `NotificationStack`, ticks it every frame, and draws it through the
// same UI `Context` as the immediate-mode menus, so the magenta-accent
// shader pass colors the frame just like our modal dialogs.

#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

namespace hg::ui
{

struct Context;

struct Notification
{
    sf::base::String text;
    float            age{0.f};      //!< seconds since spawn
    float            lifetime{3.f}; //!< seconds before removal (fade in + hold + fade out)
};

struct NotificationStack
{
    sf::base::Vector<Notification> items;
};

// Append a new toast. Idempotent only by content -- pushing the same
// text repeatedly stacks more toasts. `lifetime` is the total time
// (fade-in + hold + fade-out); pass a smaller value for transient
// updates that should disappear quickly.
void pushNotification(NotificationStack& stack, sf::base::StringView text, float lifetime = 3.f);

// Advance ages by `dt` (seconds). Removes anything past its lifetime.
// Cheap; safe to call every frame even when the stack is empty.
void tickNotifications(NotificationStack& stack, float dt);

// Draws every active notification stacked from the bottom-right of the
// `ctx`'s target, newest at the bottom. Uses `ctx.font` and the same
// `ctx.renderStates` view+transform the rest of the UI uses, so toasts
// sit in the correct virtual coordinate space and pass through the
// menu's accent-gradient shader on composite. No-op when `ctx.target`
// or `ctx.font` is null.
void drawNotifications(Context& ctx, const NotificationStack& stack);

} // namespace hg::ui
