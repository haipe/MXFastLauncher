#pragma once

#include "ObjContainerUtil.h"

#define MVPInterface  struct

#define SAFE_CLOSEWND(wnd)                  \
if ((wnd))                                  \
{                                           \
    ::SendMessage((wnd), WM_CLOSE, 0, 0);   \
    (wnd) = 0;                              \
}    

template<typename Obj = void>
MVPInterface BaseModel : public commonutil::ObjContainer<Obj>
{
    typedef Obj*    ObjPtr;
    typedef Obj     ModelObjContainerType;
    typedef ModelObjContainerType*  ModelObjContainerTypePtr;

    virtual ~BaseModel() {};

    virtual bool InitModel(ModelObjContainerTypePtr obj) = 0;
    virtual void DestoryModel() = 0;
};

MVPInterface BaseView
{
    virtual ~BaseView() {};

    virtual HWND InitView(HWND parent) = 0;

    virtual void DestoryView() = 0;
};

template<typename Model, typename View>
MVPInterface BasePresenter
{
public:
    virtual ~BasePresenter() {};
    
    typedef Model*	ModelPtr;
    typedef View*	ViewPtr;

    virtual bool	InitModel(typename Model::ModelObjContainerTypePtr model) = 0;
    virtual ViewPtr	InitView(HWND parent) = 0;
    
    virtual HWND	GetViewWnd() { return m_viewWnd; }

    virtual void	Destroy()
    {
        auto view = dynamic_cast<BaseView*>(m_view);
        if (view)view->DestoryView();

        HWND wnd = GetViewWnd();
        SAFE_CLOSEWND(wnd);
        SetViewWnd(nullptr);

        if (m_model)
            m_model->Release();
    }
    
protected:
    virtual HWND    SetViewWnd(HWND wnd) { return m_viewWnd = wnd; }

protected:
    ModelPtr	m_model = nullptr;

    ViewPtr		m_view = nullptr;
    HWND        m_viewWnd = nullptr;
};


