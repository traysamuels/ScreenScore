#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class DetachedVideoWindow : public juce::DocumentWindow {
public:
    DetachedVideoWindow(const juce::String& name, std::function<void()> onCloseCallback)
        : DocumentWindow(name,
                         juce::Colour(0xff121214),
                         DocumentWindow::allButtons),
          onClose(onCloseCallback)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setResizeLimits(320, 180, 3840, 2160);
    }

    void closeButtonPressed() override {
        if (onClose) onClose();
    }

private:
    std::function<void()> onClose;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetachedVideoWindow)
};
