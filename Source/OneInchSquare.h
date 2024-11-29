// Copyright Sharundaar All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "OneInchSquare.generated.h

// A square that's exactly one inch, finds the monitor this widget is rendered into
// read EDID data to grab the monitor's physical size
// Deduce accurate DPI, use that to render exactly one physical inch worth of pixels
UCLASS()
class UOneInchSquare : public UWidget
{
    GENERATED_BODY()

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
