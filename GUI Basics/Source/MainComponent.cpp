//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

MainComponent::MainComponent() {
    addAndMakeVisible(clearButton);
    addAndMakeVisible(transparencyLabel);
    addAndMakeVisible(transparencySlider);
    addAndMakeVisible(colorMenu);
    addAndMakeVisible(fontSizeLabel);
    addAndMakeVisible(fontSizeSlider);
    addAndMakeVisible(fontStyleLabel);
    addAndMakeVisible(boldStyleButton);
    addAndMakeVisible(italicStyleButton);
    addAndMakeVisible(editTextButton);
    addAndMakeVisible(textEditor);
    
    clearButton.addListener(this);
    transparencySlider.addListener(this);
    fontSizeSlider.addListener(this);
    boldStyleButton.addListener(this);
    italicStyleButton.addListener(this);
    editTextButton.addListener(this);
    colorMenu.addListener(this);
    
    colorMenu.addItemList(colorsStrings, 1);
    colorMenu.setTextWhenNothingSelected("Colors");
    
    transparencySlider.setRange(0.0, 1.0);
    
    fontSizeSlider.setSliderStyle(Slider::IncDecButtons);
    fontSizeSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 30, fontSizeSlider.getTextBoxHeight());
    fontSizeSlider.setRange(8.0, 32.0, 1.0);
    fontSizeSlider.setValue(textFont.getHeight(), NotificationType::dontSendNotification);
    
    textEditor.setFont(textFont);
    textEditor.setMultiLine(true);
    textEditor.setReturnKeyStartsNewLine(true);
    textEditor.setReadOnly(true);
    
    // Technically this isn't necessary here because our component will be the window's content component and the window will automatically do this for us. However, in all other cases you'll need to explicity set a component's visbility to true before you can see it!
    setVisible(true);
}

MainComponent::~MainComponent() {
}

//==============================================================================
// Component overrides

void MainComponent::paint (Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
    Rectangle<int> te = textEditor.getBounds();
    g.fillCheckerBoard(Rectangle<float>(te.getX(), te.getY(), te.getWidth(), te.getHeight()), te.getWidth()/9.0, te.getHeight()/9.0, Colours::lightgrey, Colours::white);
}

void MainComponent::resized() {
    // 8 pixel padding on all sides of MainComponent (i.e. all subcomponents should be inset by 8 pixels from the four edges)
    auto padding = 8;
    auto area = getLocalBounds().reduced(padding, padding);
    
    // Height for of all subcomponents except the textBox
    auto height = 24;
    auto clearButtonWidth = 120;
    auto colorMenuWidth = 100;
    auto transparencyLabelWidth = 100;
    auto fontSizeLabelWidth = 70;
    auto fontSizeSliderWidth = 80;
    auto fontStyleLabelWidth = 80;
    auto boldStyleButtonWidth = 60;
    auto italicStyleButtonWidth = 60;
    auto editTextButtonWidth = 86;
    
    // BOTH VERSION 1 AND VERSION 2 ACCOMPLISH THE SAME EXACT THING
    
    // VERSION 1: Only uses reduced() and removeFrom() methods
    clearButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromLeft(clearButtonWidth));
    colorMenu.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromRight(area.getWidth()-clearButton.getRight()).removeFromLeft(colorMenuWidth));
    transparencyLabel.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromRight(area.getWidth()-colorMenu.getRight()).removeFromLeft(transparencyLabelWidth));
    transparencySlider.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromRight(area.getWidth()+padding-transparencyLabel.getRight()));
    fontSizeLabel.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromLeft(fontSizeLabelWidth));
    fontSizeSlider.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()+padding-fontSizeLabel.getRight()).removeFromLeft(fontSizeSliderWidth));
    fontStyleLabel.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()-2*padding-fontSizeSlider.getRight()).removeFromLeft(fontStyleLabelWidth));
    boldStyleButton.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()+padding-fontStyleLabel.getRight()).removeFromLeft(boldStyleButtonWidth));
    italicStyleButton.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(area.getWidth()+padding-boldStyleButton.getRight()).removeFromLeft(italicStyleButtonWidth));
    editTextButton.setBounds(getLocalBounds().reduced(padding, 2*padding+height).removeFromTop(height).removeFromRight(editTextButtonWidth));
    textEditor.setBounds(getLocalBounds().reduced(padding, padding).removeFromBottom(area.getHeight()-2*padding-2*height));
    
    // VERSION 2: Uses reduced(), removeFrom(), and withTrimmed() methods
    /** clearButton.setBounds(getLocalBounds().reduced(padding, padding).removeFromTop(height).removeFromLeft(clearButtonWidth));
    colorMenu.setBounds(getLocalBounds().withTrimmedTop(padding).withTrimmedLeft(clearButton.getRight()+padding).removeFromTop(height).removeFromLeft(colorMenuWidth));
    transparencyLabel.setBounds(getLocalBounds().withTrimmedTop(padding).withTrimmedLeft(colorMenu.getRight()+padding).removeFromTop(height).removeFromLeft(transparencyLabelWidth));
    transparencySlider.setBounds(getLocalBounds().withTrimmedTop(padding).withTrimmedRight(padding).withTrimmedLeft(transparencyLabel.getRight()).removeFromTop(height));
    fontSizeLabel.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedLeft(padding).removeFromTop(height).removeFromLeft(fontSizeLabelWidth));
    fontSizeSlider.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedLeft(fontSizeLabel.getRight()).removeFromTop(height).removeFromLeft(fontSizeSliderWidth));
    fontStyleLabel.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedLeft(fontSizeSlider.getRight()+3*padding).removeFromTop(height).removeFromLeft(fontStyleLabelWidth));
    boldStyleButton.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedLeft(fontStyleLabel.getRight()).removeFromTop(height).removeFromLeft(boldStyleButtonWidth));
    italicStyleButton.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedLeft(boldStyleButton.getRight()).removeFromTop(height).removeFromLeft(italicStyleButtonWidth));
    editTextButton.setBounds(getLocalBounds().withTrimmedTop(clearButton.getBottom()+padding).withTrimmedRight(padding).removeFromTop(height).removeFromRight(editTextButtonWidth));
    textEditor.setBounds(getLocalBounds().reduced(padding, padding).withTrimmedTop(fontSizeLabel.getBottom())); */
}
//
//==============================================================================
// Listener overrides

void MainComponent::buttonClicked(Button* button ) {
    if (button == &clearButton) {
        textEditor.clear();
        repaint();
    } else if (button == &boldStyleButton) {
        Font font = textEditor.getFont();
        if (boldStyleButton.getToggleState()) {
            font.setStyleFlags(font.getStyleFlags() | Font::bold);
        } else {
            // 6 is 110 in binary, so bitwise and-ing font style flags with 110 will unbold the font.
            font.setStyleFlags(font.getStyleFlags() & 6);
        }
        textEditor.applyFontToAllText(font, true);
    } else if (button == &italicStyleButton) {
        Font font = textEditor.getFont();
        if (italicStyleButton.getToggleState()) {
            font.setStyleFlags(font.getStyleFlags() | Font::italic);
        } else {
            // 5 is 101 in binary, so bitwise and-ing font style flags with 101 will unitalicize the font.
            font.setStyleFlags(font.getStyleFlags() & 5);
        }
        textEditor.applyFontToAllText(font, true);
    } else if (button == &editTextButton) {
        if (editTextButton.getToggleState()) {
            textEditor.setReadOnly(false);
        } else {
            textEditor.setReadOnly(true);
        }
    }
}

void MainComponent::sliderValueChanged(Slider* slider) {
    if (slider == &transparencySlider) {
        // As transparencySlider's value moves from 0.0 to 1.0 the textEditor's opacity moves from 1.0 to 0.0.
        float opacityMapped = jmap(transparencySlider.getValue(), 0.0, 1.0, 1.0, 0.0);
        textEditor.setAlpha(opacityMapped);
    } else if (slider == &fontSizeSlider) {
        Font font = textEditor.getFont();
        font.setHeight(fontSizeSlider.getValue());
        textEditor.applyFontToAllText(font, true);
    }
}

void MainComponent::comboBoxChanged(ComboBox* menu) {
    if (menu == &colorMenu) {
        textEditor.setColour(TextEditor::ColourIds::backgroundColourId, colors[menu->getSelectedItemIndex()]);
        repaint();
    }
}

//==============================================================================
// MainComponent methods

void MainComponent::setTextForId(const int id) {
    if (id == MainApplication::LATIN_TEXT) {
        textEditor.setText(latinText);
    } else if (id == MainApplication::GREEK_TEXT) {
        textEditor.setText(greekText);
    } else if (id == MainApplication::CYRILLIC_TEXT) {
        textEditor.setText(cyrillicText);
    }
    repaint();
}
