/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* leash/LeashUIApplication.cpp - Implement IUIApplication for leash */
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
 * Implementation of the LeashUIApplication class.  It implements
 * only the minimum requirements for the IUIApplication interface,
 * which is required in order to use the windows framework ribbon.
 */

#include <UIRibbon.h>
#include "kfwribbon.h"
#include "LeashUIApplication.h"
#include "LeashUICommandHandler.h"

HWND LeashUIApplication::mainwin;

HRESULT
LeashUIApplication::CreateInstance(IUIApplication **out, HWND hwnd)
{
    LeashUIApplication *app;
    HRESULT ret;

    if (out == NULL)
        return E_POINTER;
    *out = NULL;

    app = new LeashUIApplication();
    if (app == NULL)
        return E_OUTOFMEMORY;
    ret = LeashUICommandHandler::CreateInstance(&app->commandHandler, hwnd);
    if (!SUCCEEDED(ret)) {
        delete app;
        return ret;
    }
    ret = app->InitializeRibbon(hwnd);
    if (FAILED(ret)) {
        delete app;
        return ret;
    }
    mainwin = hwnd;
    *out = static_cast<IUIApplication *>(app);
    return S_OK;
}

HRESULT
LeashUIApplication::InitializeRibbon(HWND hwnd)
{
    HRESULT ret;

    if (hwnd == NULL)
        return -1;
    ret = CoInitialize(NULL);
    if (FAILED(ret)) {
        //MessageBox("CoInitialize!", "Error", MB_OK);
	return -1;
    }
    ret = CoCreateInstance(CLSID_UIRibbonFramework, NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_PPV_ARGS(&ribbonFramework));
    if (FAILED(ret)) {
        //MessageBox("CoCreateInstance!", "Error", MB_OK);
        return -1;
    }
    ret = ribbonFramework->Initialize(hwnd, this);
    if (FAILED(ret)) {
        //MessageBox("RibbonFramework::Initialize!", "Error", MB_OK);
        return -1;
    }
    ret = ribbonFramework->LoadUI(GetModuleHandle(NULL),
                                  L"KFW_RIBBON_RIBBON");
    if (FAILED(ret)) {
        //MessageBox("RibbonFramework::LoadUI!", "Error", MB_OK);
        return -1;
    }
    return S_OK;
}

ULONG
LeashUIApplication::AddRef()
{
    return InterlockedIncrement(&refcnt);
}

ULONG
LeashUIApplication::Release()
{
    LONG tmp;

    tmp = InterlockedDecrement(&refcnt);
    if (tmp == 0) 
        delete this;
    return tmp;
}

HRESULT
LeashUIApplication::QueryInterface(REFIID iid, void **ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    if (iid == __uuidof(IUnknown)) {
        *ppv = static_cast<IUnknown*>(this);
    } else if (iid == __uuidof(IUIApplication)) {
        *ppv = static_cast<IUIApplication*>(this);
    } else {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT
LeashUIApplication::OnViewChanged(UINT32 viewId, UI_VIEWTYPE typeID,
                                  IUnknown *view, UI_VIEWVERB verb,
                                  INT32 uReasonCode)
{
    IUIRibbon *ribbon;
    HRESULT ret;

    if (viewId != 0 || typeID != UI_VIEWTYPE_RIBBON)
        return E_NOTIMPL;

    switch(verb) {
        case UI_VIEWVERB_DESTROY:
            /* We didn't cache any state at create time, so do nothing. */
            return S_OK;
        case UI_VIEWVERB_CREATE:
            /* Fall through to get the initial ribbon height. */
        case UI_VIEWVERB_SIZE:
            ret = view->QueryInterface(IID_PPV_ARGS(&ribbon));
            if (!SUCCEEDED(ret))
                return ret;
            ret = ribbon->GetHeight(&ribbonHeight);
            if (!SUCCEEDED(ret)) {
                ribbon->Release();
                return ret;
            }
            if (verb == UI_VIEWVERB_SIZE) {
		// Tell the main frame to recalc its layout and redraw
                // 0x0368 is WM_RECALCPARENT, but we don't want that header
		SendMessage(mainwin, 0x0368, 0, NULL);
            }
            //MessageBox("Got a new ribbon height", "Info", MB_OK);
            ribbon->Release();
            return S_OK;
        case UI_VIEWVERB_ERROR:
            /* fallthrough */
        default:
            return E_NOTIMPL;
    }
}

/*
 * Provide a command handler to which the command with ID commandId will
 * be bound.  All of our commands get the same handler.
 *
 * The typeID argument is just an enum which classifies what type of
 * command this is, grouping types of buttons together, collections,
 * etc.  Since we only have one command handler, it can safely be ignored.
 */
HRESULT
LeashUIApplication::OnCreateUICommand(UINT32 commandId, UI_COMMANDTYPE typeID,
                                      IUICommandHandler **commandHandler)
{
    return this->commandHandler->QueryInterface(IID_PPV_ARGS(commandHandler));
}

/*
 * It looks like this is called by the framework when the window with the
 * ribbon is going away, to give the application a chance to free any
 * application-specific resources (not from the framwork) that were bound
 * to a command in OnCreateUICommand.
 *
 * We do not have any such resources, so we do not need to implement this
 * function.  I'm not sure whether it would be better to return S_OK, though.
 */
HRESULT
LeashUIApplication::OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeID,
                                       IUICommandHandler *commandHandler)
{
    return E_NOTIMPL;
}
