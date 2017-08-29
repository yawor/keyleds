/* Keyleds -- Gaming keyboard tool
 * Copyright (C) 2017 Julien Hartmann, juli1.hartmann@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtCore>
#include "keyledsd/ContextWatcher.h"
#include "logging.h"

LOGGING("context-watcher");

using keyleds::XContextWatcher;
const std::string activeWindowAtom = "_NET_ACTIVE_WINDOW";

/****************************************************************************/

XContextWatcher::XContextWatcher(QObject *parent)
 : XContextWatcher(std::string(), parent) {}

XContextWatcher::XContextWatcher(std::string display, QObject *parent)
 : QObject(parent),
   m_display(display)
{
    setupDisplay(m_display);
    // this QObject takes automatic ownership of QSocketNotifier
    auto notifier = new QSocketNotifier(m_display.connection(), QSocketNotifier::Read, this);
    QObject::connect(notifier, SIGNAL(activated(int)),
                     this, SLOT(onDisplayEvent(int)));
}

XContextWatcher::XContextWatcher(xlib::Display && display, QObject *parent)
 : QObject(parent),
   m_display(std::move(display))
{
    setupDisplay(m_display);
    // this QObject takes automatic ownership of QSocketNotifier
    auto notifier = new QSocketNotifier(m_display.connection(), QSocketNotifier::Read, this);
    QObject::connect(notifier, SIGNAL(activated(int)),
                     this, SLOT(onDisplayEvent(int)));
}

void XContextWatcher::setupDisplay(xlib::Display & display)
{
    XSetWindowAttributes attributes;
    attributes.event_mask = PropertyChangeMask;
    display.root().changeAttributes(CWEventMask, attributes);

    m_activeWindow = display.getActiveWindow();
    onActiveWindowChanged(m_activeWindow.get());
}

void XContextWatcher::onDisplayEvent(int)
{
    while (XPending(m_display.handle())) {
        XEvent event;
        XNextEvent(m_display.handle(), &event);
        handleEvent(event);
    }
}

void XContextWatcher::handleEvent(XEvent & event)
{
    switch (event.type) {
    case PropertyNotify:
        if (event.xproperty.atom == m_display.atom(activeWindowAtom)) {
            auto active = m_display.getActiveWindow();
            if ((active == nullptr) != (m_activeWindow == nullptr) ||
                (active != nullptr && active->handle() != m_activeWindow->handle())) {
                // Event must fire before the property is updated
                onActiveWindowChanged(active.get());
                m_activeWindow = std::move(active);
            }
        }

        if (m_activeWindow != nullptr && (
                event.xproperty.atom == m_display.atom("_NET_WM_NAME") ||
                event.xproperty.atom == m_display.atom("WM_NAME"))) {

            xlib::ErrorCatcher errors;

            setContext(m_activeWindow.get());

            errors.synchronize(m_display);
            if (errors) {
                DEBUG("PropertNotify - _NET_WM_NAME changed had ", errors.errors().size(), " errors");
                m_activeWindow = nullptr;
            }
        }
        break;
    }
}

void XContextWatcher::onActiveWindowChanged(xlib::Window * window)
{
    xlib::ErrorCatcher errors;

    if (m_activeWindow != nullptr) {
        XSetWindowAttributes attributes;
        attributes.event_mask = NoEventMask;
        m_activeWindow->changeAttributes(CWEventMask, attributes);
    }

    if (window != nullptr) {
        XSetWindowAttributes attributes;
        attributes.event_mask = PropertyChangeMask;
        window->changeAttributes(CWEventMask, attributes);
    }
    setContext(window);

    errors.synchronize(m_display);
    if (errors) {
        DEBUG("onActiveWindowChanged, ignoring ", errors.errors().size(), " errors");
    }
}

void XContextWatcher::setContext(xlib::Window * window)
{
    Context context;
    if (window == nullptr) {
        context = {
            { "id", std::string() },
            { "title", std::string() },
            { "class", std::string() },
            { "instance", std::string() }
        };
    } else {
        context = {
            { "id", std::to_string(window->handle()) },
            { "title", window->name() },
            { "class", window->className() },
            { "instance", window->instanceName() }
        };
    }

    if (context != m_context) {
        m_context = std::move(context);
        emit contextChanged(m_context);
    }
}
