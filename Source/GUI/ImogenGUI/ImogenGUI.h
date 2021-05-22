/*======================================================================================================================================================
           _             _   _                _                _                 _               _
          /\ \          /\_\/\_\ _           /\ \             /\ \              /\ \            /\ \     _
          \ \ \        / / / / //\_\        /  \ \           /  \ \            /  \ \          /  \ \   /\_\
          /\ \_\      /\ \/ \ \/ / /       / /\ \ \         / /\ \_\          / /\ \ \        / /\ \ \_/ / /
         / /\/_/     /  \____\__/ /       / / /\ \ \       / / /\/_/         / / /\ \_\      / / /\ \___/ /
        / / /       / /\/________/       / / /  \ \_\     / / / ______      / /_/_ \/_/     / / /  \/____/
       / / /       / / /\/_// / /       / / /   / / /    / / / /\_____\    / /____/\       / / /    / / /
      / / /       / / /    / / /       / / /   / / /    / / /  \/____ /   / /\____\/      / / /    / / /
  ___/ / /__     / / /    / / /       / / /___/ / /    / / /_____/ / /   / / /______     / / /    / / /
 /\__\/_/___\    \/_/    / / /       / / /____\/ /    / / /______\/ /   / / /_______\   / / /    / / /
 \/_________/            \/_/        \/_________/     \/___________/    \/__________/   \/_/     \/_/
 
 
 This file is part of the Imogen codebase.
 
 @2021 by Ben Vining. All rights reserved.
 
 ImogenGUI.h: This file defines the interface for Imogen's top-level GUI component. This class does not reference ImogenAudioProcessor, so that it can also be used to  create a GUI-only remote control application for Imogen.
 
======================================================================================================================================================*/


/*******************************************************************************
 BEGIN_JUCE_MODULE_DECLARATION
 ID:                 ImogenEngine
 vendor:             Ben Vining
 version:            0.0.1
 name:               ImogenEngine
 description:        self-contained module representing Imogen's entire user interface
 dependencies:       bv_gui ImogenCommon
 END_JUCE_MODULE_DECLARATION
 *******************************************************************************/


#pragma once

#include "bv_gui/bv_gui.h"
#include "ImogenCommon/ImogenCommon.h"

#include "LookAndFeel/ImogenLookAndFeel.h"
#include "MainDialComponent/MainDialComponent.h"

#include "BinaryData.h"


struct ImogenGUIUpdateSender
{
    virtual void sendValueTreeStateChange (const void* encodedChange, size_t encodedChangeSize) = 0;
};

/*=========================================================================================*/


class ImogenGUI : public juce::Component,
                  public bav::GUIInitializer
{
    using ParameterID               = Imogen::ParameterID;
    using MeterID                   = Imogen::MeterID;
    using NonAutomatableParameterID = Imogen::NonAutomatableParameterID;

public:
    ImogenGUI (ImogenGUIUpdateSender* s);

    virtual ~ImogenGUI() override;

    /*=========================================================================================*/

    void applyValueTreeStateChange (const void* encodedChangeData, size_t encodedChangeDataSize);

    /*=========================================================================================*/
    /* juce::Component functions */

    void paint (juce::Graphics& g) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;
    void modifierKeysChanged (const juce::ModifierKeys& modifiers) override;
    void focusLost (FocusChangeType cause) override;

    /*=========================================================================================*/

    void setDarkMode (bool shouldUseDarkMode);
    bool isUsingDarkMode() const noexcept { return darkMode.load(); }

    /*=========================================================================================*/

private:
    inline void makePresetMenu (juce::ComboBox& box);

    void rescanPresetsFolder();
    void loadPreset (const juce::String& presetName);
    void savePreset (const juce::String& presetName);
    void renamePreset (const juce::String& previousName, const juce::String& newName);
    void deletePreset (const juce::String& presetName);

    /*=========================================================================================*/

    bav::NonParamValueTreeNode* getPropertyPntr (const NonAutomatableParameterID propID) const;

    bav::FreestandingParameter* getParameterPntr (const ParameterID paramID) const;

    bav::FreestandingParameter* getMeterPntr (const MeterID meterID) const;

    /*=========================================================================================*/

    juce::UndoManager undoManager;

    std::vector< bav::NonParamValueTreeNode* > propertyPointers;

    juce::ValueTree state;

    struct ValueTreeSynchronizer : public juce::ValueTreeSynchroniser
    {
        ValueTreeSynchronizer (const juce::ValueTree& vtree, ImogenGUIUpdateSender* s)
            : juce::ValueTreeSynchroniser (vtree)
            , sender (s)
        {
        }

        void stateChanged (const void* encodedChange, size_t encodedChangeSize) override final
        {
            sender->sendValueTreeStateChange (encodedChange, encodedChangeSize);
        }

        ImogenGUIUpdateSender* const sender;
    };

    ValueTreeSynchronizer treeSync;

    std::unique_ptr< bav::NonParamValueTreeNodeGroup > properties;

    juce::OwnedArray< bav::FreestandingParameter > parameters;
    juce::OwnedArray< bav::FreestandingParameter > meters;

    juce::OwnedArray< bav::PropertyAttachmentBase >                     propertyValueTreeAttachments; // these are two-way
    juce::OwnedArray< bav::FreeStandingParameterAttachment >            parameterValueTreeAttachments; // these are two-way
    juce::OwnedArray< bav::ValueTreeToFreeStandingParameterAttachment > meterValueTreeAttachments; // these are read-only

    /*=========================================================================================*/

    ImogenDialComponent mainDial;

    bav::ImogenLookAndFeel lookAndFeel;

    juce::TooltipWindow  tooltipWindow;
    static constexpr int msBeforeTooltip = 700;

    std::atomic< bool > darkMode;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImogenGUI)
};
