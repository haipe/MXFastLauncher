#pragma once

#include "BaseMVP.h"
#include "CommonUtil.h"

MVPInterface FLMainToastView : public BaseView
{
    virtual ~FLMainToastView() {}

    MVPInterface Notify
   {
        virtual int OnAdd(const TString& path) = 0;
    };
    
    virtual void Show(bool show) = 0;
};