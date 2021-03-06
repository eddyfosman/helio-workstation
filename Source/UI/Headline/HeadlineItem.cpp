/*
    This file is part of Helio Workstation.

    Helio is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Helio is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Helio. If not, see <http://www.gnu.org/licenses/>.
*/

//[Headers]
#include "Common.h"
//[/Headers]

#include "HeadlineItem.h"

//[MiscUserDefs]
#include "IconComponent.h"
#include "PanelBackgroundB.h"
#include "Headline.h"
#include "HeadlineDropdown.h"
#include "CachedLabelImage.h"
#include "ColourIDs.h"
#include "MainLayout.h"

class HeadlineContextMenuMarker final : public Component
{
public:

    HeadlineContextMenuMarker() :
        dark(findDefaultColour(ColourIDs::Common::borderLineDark).withMultipliedAlpha(0.5f)),
        light(findDefaultColour(ColourIDs::Common::borderLineLight).withMultipliedAlpha(2.f))
    {
        this->setPaintingIsUnclipped(true);
        this->setInterceptsMouseClicks(false, false);
        this->setWantsKeyboardFocus(false);
        this->setSize(1, 3);
        this->setAlpha(0.f);
    }

    void paint(Graphics &g) override
    {
        static constexpr auto dashLength = 8;
        static constexpr auto s = dashLength - 2;
        const auto width = this->getWidth() - s;
        const auto w = width - (width % dashLength);

        g.setColour(this->dark);
        g.fillRect(s + 1, 1, w, 1);
        g.fillRect(s, 2, w, 1);

        g.setColour(this->light);
        for (int i = s; i < w; i += (dashLength * 2))
        {
            g.fillRect(i + 1, 1, dashLength, 1);
            g.fillRect(i, 2, dashLength, 1);
        }
    }

    const Colour dark;
    const Colour light;
};

//[/MiscUserDefs]

HeadlineItem::HeadlineItem(WeakReference<HeadlineItemDataSource> treeItem, AsyncUpdater &parent)
    : item(treeItem),
      parentHeadline(parent)
{
    this->titleLabel.reset(new Label(String(),
                                            String()));
    this->addAndMakeVisible(titleLabel.get());
    this->titleLabel->setFont(Font (18.00f, Font::plain));
    titleLabel->setJustificationType(Justification::centredLeft);
    titleLabel->setEditable(false, false, false);

    this->icon.reset(new IconComponent(Icons::helio));
    this->addAndMakeVisible(icon.get());

    this->arrow.reset(new HeadlineItemArrow());
    this->addAndMakeVisible(arrow.get());

    //[UserPreSize]
    this->menuMarker = make<HeadlineContextMenuMarker>();
    this->addChildComponent(this->menuMarker.get());

    this->titleLabel->setInterceptsMouseClicks(false, false);
    this->setInterceptsMouseClicks(true, true);
    this->setMouseClickGrabsKeyboardFocus(false);
    this->setPaintingIsUnclipped(true);
    this->setOpaque(false);

    this->titleLabel->setBufferedToImage(true);
    this->titleLabel->setCachedComponentImage(new CachedLabelImage(*this->titleLabel));

    this->bgColour = findDefaultColour(ColourIDs::BackgroundA::fill);
    //[/UserPreSize]

    this->setSize(256, 32);

    //[Constructor]
    if (this->item != nullptr)
    {
        this->item->addChangeListener(this);
    }
    //[/Constructor]
}

HeadlineItem::~HeadlineItem()
{
    //[Destructor_pre]
    this->stopTimer();

    if (this->item != nullptr)
    {
        this->item->removeChangeListener(this);
    }
    //[/Destructor_pre]

    titleLabel = nullptr;
    icon = nullptr;
    arrow = nullptr;

    //[Destructor]
    //[/Destructor]
}

void HeadlineItem::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    {
        Colour fillColour = this->bgColour;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //fillColour = this->bgColour;
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillPath (internalPath1);
    }

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void HeadlineItem::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    this->menuMarker->setBounds(Headline::itemsOverlapOffset,
        this->getHeight() - this->menuMarker->getHeight(),
        this->getWidth() - Headline::itemsOverlapOffset,
        this->menuMarker->getHeight());
    //[/UserPreResize]

    titleLabel->setBounds(33, (getHeight() / 2) - (30 / 2), 256, 30);
    icon->setBounds(12, (getHeight() / 2) - (26 / 2), 26, 26);
    arrow->setBounds(getWidth() - 16, 0, 16, getHeight() - 0);
    internalPath1.clear();
    internalPath1.startNewSubPath (2.0f, 1.0f);
    internalPath1.lineTo (static_cast<float> (getWidth() - 16), 1.0f);
    internalPath1.lineTo (static_cast<float> (getWidth() - 2), static_cast<float> (getHeight() - 2));
    internalPath1.lineTo (1.0f, static_cast<float> (getHeight() - 1));
    internalPath1.lineTo (2.0f, static_cast<float> (getHeight() - 2));
    internalPath1.closeSubPath();

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void HeadlineItem::mouseEnter (const MouseEvent& e)
{
    //[UserCode_mouseEnter] -- Add your code here...
#if HELIO_DESKTOP
    // A hacky way to prevent re-opening the menu again after the new page is shown.
    // Showhow comparing current mouse screen position to e.getMouseDownScreenPosition()
    // won't work (maybe a JUCE bug), so get it from getMainMouseSource:
    const auto lastMouseDown =
        Desktop::getInstance().getMainMouseSource().getLastMouseDownPosition().toInt();
    if (lastMouseDown != e.getScreenPosition())
    {
        this->startTimer(100);
    }
#endif
    //[/UserCode_mouseEnter]
}

void HeadlineItem::mouseExit (const MouseEvent& e)
{
    //[UserCode_mouseExit] -- Add your code here...
    this->stopTimer();
    //[/UserCode_mouseExit]
}

void HeadlineItem::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    if (this->item != nullptr)
    {
        this->stopTimer();

        // on desktop versions, as quick click on a headline item opens its node's page,
        // on mobile versions, it always opens the menu first
#if HELIO_DESKTOP
        if (this->item->canBeSelectedAsMenuItem())
        {
            this->item->onSelectedAsMenuItem();
        }
        else
        {
            this->showMenuIfAny();
        }
#elif HELIO_MOBILE
        this->showMenuIfAny();
#endif
    }
    //[/UserCode_mouseDown]
}

void HeadlineItem::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    this->stopTimer();
    //[/UserCode_mouseUp]
}


//[MiscUserCode]

WeakReference<HeadlineItemDataSource> HeadlineItem::getDataSource() const noexcept
{
    return this->item;
}

void HeadlineItem::updateContent()
{
    if (this->item != nullptr)
    {
        this->icon->setIconImage(this->item->getIcon());
        this->titleLabel->setText(this->item->getName(), dontSendNotification);
        const int textWidth = this->titleLabel->getFont().getStringWidth(this->titleLabel->getText());
        const int maxTextWidth = this->titleLabel->getWidth();
        this->setSize(jmin(textWidth, maxTextWidth) + 46 + Headline::itemsOverlapOffset, Globals::UI::headlineHeight - 1);
    }
}

void HeadlineItem::changeListenerCallback(ChangeBroadcaster *source)
{
    this->parentHeadline.triggerAsyncUpdate();
}

void HeadlineItem::timerCallback()
{
    this->stopTimer();
    this->showMenuIfAny();
}

void HeadlineItem::showMenuIfAny()
{
    if (this->item != nullptr && this->item->hasMenu())
    {
        App::showModalComponent(make<HeadlineDropdown>(this->item, this->getPosition()));
    }
}

void HeadlineItem::showContextMenuMarker()
{
    this->animator.fadeIn(this->menuMarker.get(), Globals::UI::fadeInLong);
    //this->menuMarker->setVisible(true);
}

void HeadlineItem::hideContextMenuMarker()
{
    this->animator.fadeOut(this->menuMarker.get(), Globals::UI::fadeOutLong);
    //this->menuMarker->setVisible(false);
}
//[/MiscUserCode]

#if 0
/*
BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="HeadlineItem" template="../../Template"
                 componentName="" parentClasses="public Component, private Timer, private ChangeListener"
                 constructorParams="WeakReference&lt;HeadlineItemDataSource&gt; treeItem, AsyncUpdater &amp;parent"
                 variableInitialisers="item(treeItem),&#10;parentHeadline(parent)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="256" initialHeight="32">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseEnter (const MouseEvent&amp; e)"/>
    <METHOD name="mouseExit (const MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0">
    <PATH pos="0 0 100 100" fill="solid: 15ffffff" hasStroke="0" nonZeroWinding="1">s 2 1 l 16R 1 l 2R 2R l 1 1R l 2 2R x</PATH>
  </BACKGROUND>
  <LABEL name="" id="9a3c449859f61884" memberName="titleLabel" virtualName=""
         explicitFocusOrder="0" pos="33 0Cc 256 30" labelText="" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="18.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="" id="f10feab7d241bacb" memberName="icon" virtualName=""
                    explicitFocusOrder="0" pos="12 0Cc 26 26" class="IconComponent"
                    params="Icons::helio"/>
  <JUCERCOMP name="" id="6845054f3705e31" memberName="arrow" virtualName=""
             explicitFocusOrder="0" pos="0Rr 0 16 0M" sourceFile="HeadlineItemArrow.cpp"
             constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



