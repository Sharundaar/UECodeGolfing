// Copyright Sharundaar All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Engine/UserInterfaceSettings.h"

#include "UIMetrics.generated.h"

USTRUCT(BlueprintType)
struct FUIMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	UWidget* Widget = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FText WindowTitle = INVTEXT("");

	UPROPERTY(BlueprintReadWrite)
	FVector2D WindowSize = FVector2D(0);

	UPROPERTY(BlueprintReadWrite)
	FVector2D WindowPosition = FVector2D(0);

	UPROPERTY(BlueprintReadWrite)
	float WindowTitleBarSize = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FMargin WindowBorderSize = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float WindowDPI = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector2D ViewportSize = FVector2D(0);
	
	UPROPERTY(BlueprintReadWrite)
	float ViewportDPI = 0.0f;
	
	UPROPERTY(BlueprintReadWrite)
	float ApplicationScale = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FGeometry WidgetGeometry;

	UPROPERTY(BlueprintReadWrite)
	FGeometry SelfGeometry;

	UPROPERTY(BlueprintReadWrite)
	FVector2D WindowContentScale;
};

UCLASS()
class UMetricWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	FUIMetrics GenerateSelfMetrics();

	UFUNCTION(BlueprintCallable)
	static FUIMetrics GenerateMetrics(UWidget* ForWidget);
};

inline void UMetricWidget::NativeConstruct()
{
	Super::NativeConstruct();

	FSlateThrottleManager::Get().DisableThrottle(true);
}

inline FUIMetrics UMetricWidget::GenerateSelfMetrics()
{
	return GenerateMetrics(this);
}

inline FUIMetrics UMetricWidget::GenerateMetrics(UWidget* ForWidget)
{
	FUIMetrics Metrics = {};

	Metrics.Widget = ForWidget;
	if (Metrics.Widget)
	{
		Metrics.WidgetGeometry = Metrics.Widget->GetCachedGeometry();
	}

	Metrics.SelfGeometry = ForWidget ? ForWidget->GetCachedGeometry() : FGeometry{};

	const FSlateApplication& SlateApplication = FSlateApplication::Get();
	
	FDisplayMetrics DisplayMetrics;
	SlateApplication.GetInitialDisplayMetrics(DisplayMetrics);
	Metrics.ApplicationScale = SlateApplication.GetApplicationScale();
	TSharedPtr<SWindow> WidgetWindow = nullptr;
	if (TSharedPtr<SWidget> CachedWidget = ForWidget ? ForWidget->GetCachedWidget() : nullptr)
	{
		while (CachedWidget)
		{
			if (CachedWidget->Advanced_IsWindow())
			{
				WidgetWindow = StaticCastSharedPtr<SWindow>(CachedWidget);
				break;
			}
			CachedWidget = CachedWidget->GetParentWidget();
		}
	}

	if (WidgetWindow)
	{
		Metrics.WindowTitle = WidgetWindow->GetTitle();
		Metrics.WindowSize = WidgetWindow->GetSizeInScreen();
		Metrics.WindowPosition = WidgetWindow->GetPositionInScreen();
		Metrics.WindowTitleBarSize = WidgetWindow->GetTitleBarSize().Get();
		Metrics.WindowBorderSize = FMargin(WidgetWindow->GetNativeWindow()->GetWindowBorderSize());
		Metrics.WindowContentScale = WidgetWindow->GetContentScale();

		const UUserInterfaceSettings* UserInterfaceSettings = GetDefault<UUserInterfaceSettings>();
		Metrics.ViewportSize = WidgetWindow->GetViewport() ? WidgetWindow->GetViewport()->GetSize() : FVector2D(0);
		Metrics.ViewportDPI = UserInterfaceSettings->GetDPIScaleBasedOnSize(FIntPoint(Metrics.ViewportSize.X, Metrics.ViewportSize.Y));

		Metrics.WindowDPI = WidgetWindow->GetDPIScaleFactor();
	}
	
	return Metrics;
}

UE_ENABLE_OPTIMIZATION
