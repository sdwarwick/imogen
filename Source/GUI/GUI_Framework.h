
#pragma once

#include "../Common/ImogenCommon.h"


using namespace Imogen;


/*
 */



/*
 */


//class ImogenComponent   :       public juce::Component,
//                                public ImogenGUIParameter::Listener
//{
//public:
//    ImogenComponent(ImogenEventSender* h, ImogenGUIParameter* p)
//        : holder(h), parameter(p)
//    {
//        jassert (holder != nullptr && parameter != nullptr);
//        parameter->addListener (this);
//        parameter->setComponent (this);
//    }
//
//    virtual ~ImogenComponent()
//    {
//        parameter->removeListener (this);
//
//        if (parameter->getComponent() == this)
//            parameter->setComponent (nullptr);
//    }
//
//    ImogenGUIParameter* getParameter() const noexcept { return parameter; }
//
//    virtual void setDarkMode (bool shouldUseDarkMode) { juce::ignoreUnused (shouldUseDarkMode); }
//
//protected:
//    ImogenEventSender* const holder;
//    ImogenGUIParameter* const parameter;
//};
