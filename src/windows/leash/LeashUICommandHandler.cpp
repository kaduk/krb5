/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* leash/LeashUICommandHandler.cpp - implements IUICommandHandler interfaces */
/*
 * Copyright (C) 2014 by the Massachusetts Institute of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains the class implementation of the leash implementation
 * of the UICommandHandler interface.  Its primary responsibility is
 * to accept UI events (i.e., button presses) and perform the
 * corresponding actions to the leash data structures and display
 * presentation.
 */

#include <UIRibbon.h>
#include "kfwribbon.h"
#include "LeashUICommandHandler.h"
#include "resource.h"

HRESULT
LeashUICommandHandler::CreateInstance(IUICommandHandler **out, HWND hwnd)
{
    LeashUICommandHandler *handler;

    if (out == NULL)
        return E_POINTER;

    handler = new LeashUICommandHandler();
    if (handler == NULL)
        return E_OUTOFMEMORY;
    handler->mainwin = hwnd;
    *out = static_cast<IUICommandHandler *>(handler);
    return S_OK;
}

ULONG
LeashUICommandHandler::AddRef()
{
    return InterlockedIncrement(&refcnt);
}

ULONG
LeashUICommandHandler::Release()
{
    LONG tmp;

    tmp = InterlockedDecrement(&refcnt);
    if (tmp == 0) 
        delete this;
    return tmp;
}

HRESULT
LeashUICommandHandler::QueryInterface(REFIID iid, void **ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    if (iid == __uuidof(IUnknown)) {
        *ppv = static_cast<IUnknown*>(this);
    } else if (iid == __uuidof(IUICommandHandler)) {
        *ppv = static_cast<IUICommandHandler*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT
LeashUICommandHandler::Execute(UINT32 commandId, UI_EXECUTIONVERB verb,
                               const PROPERTYKEY *key,
                               const PROPVARIANT *currentValue,
                               IUISimplePropertySet *commandExecutionProperties)
{
    if (verb != UI_EXECUTIONVERB_EXECUTE)
	return E_NOTIMPL;
    switch(commandId) {
        case cmdGetTicketButton:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_INIT_TICKET, 1), 0);
            break;
        case cmdRenewTicketButton:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_RENEW_TICKET, 1), 0);
            break;
        case cmdDestroyTicketButton:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_DESTROY_TICKET, 1),
                        0);
            break;
        case cmdMakeDefaultButton:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_MAKE_DEFAULT, 1),
                        0);
            break;
        case cmdChangePasswordButton:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_CHANGE_PASSWORD, 1),
                        0);
            break;
        case cmdIssuedCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_TIME_ISSUED, 1), 0);
            break;
        case cmdRenewUntilCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_RENEWABLE_UNTIL, 1),
                        0);
            break;
        case cmdValidUntilCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_VALID_UNTIL, 1), 0);
            break;
        case cmdEncTypeCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_ENCRYPTION_TYPE, 1),
                        0);
            break;
        case cmdFlagsCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_SHOW_TICKET_FLAGS,
                                                        1), 0);
            break;
        case cmdAutoRenewCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_AUTO_RENEW, 1), 0);
            break;
        case cmdExpireAlarmCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_LOW_TICKET_ALARM,
                                                        1), 0);
            break;
        case cmdDestroyOnExitCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_KILL_TIX_ONEXIT, 1),
                        0);
            break;
        case cmdMixedCaseCheckBox:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_UPPERCASE_REALM, 1),
                        0);
            break;
        case cmdHelp:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_HELP_LEASH32, 1), 0);
            break;
#if 0
        case cmdAbout:
            SendMessage(mainwin, WM_COMMAND, MAKEWPARAM(ID_APP_ABOUT, 1), 0);
            break;
        case cmdExit:
            SendMessage(mainwin, WM_GOODBYE, 0, 0);
            break;
#endif
        default:
            // Lots of commands we don't need to pass on
            return S_OK;
    }
    return S_OK;
}

HRESULT
LeashUICommandHandler::UpdateProperty(UINT32 commandId, REFPROPERTYKEY key,
                                      const PROPVARIANT *currentValue,
                                      PROPVARIANT *newValue)
{
    return E_NOTIMPL;
}
