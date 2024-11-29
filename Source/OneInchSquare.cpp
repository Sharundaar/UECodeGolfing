// Copyright Sharundaar All Rights Reserved.

#include "OneInchSquare.h"

THIRD_PARTY_INCLUDES_START
#include <ShlObj.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "FUIShapesModule"

class SOneInchSquare : public SLeafWidget
{
	SLATE_BEGIN_ARGS(SOneInchSquare)
	{}
	SLATE_END_ARGS()

	SOneInchSquare() {}

public:
	void Construct(const FArguments& Args);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D(DPI/LayoutScaleMultiplier, DPI/LayoutScaleMultiplier); };
	mutable HMONITOR LatestDPIComputedMonitor = NULL;
	mutable double DPI = 0.0;
};

void SOneInchSquare::Construct(const FArguments& Args)
{
}

static bool GetMonitorSizeFromEDID(const HKEY hDevRegKey, int32& OutWidth, int32& OutHeight, int32& OutDPI)
{	
	static const uint32 NameSize = 512;
	static TCHAR ValueName[NameSize];

	DWORD Type;
	DWORD ActualValueNameLength = NameSize;

	BYTE EDIDData[1024];
	DWORD EDIDSize = sizeof(EDIDData);

	for (LONG i = 0, RetValue = ERROR_SUCCESS; RetValue != ERROR_NO_MORE_ITEMS; ++i)
	{
		RetValue = RegEnumValue ( hDevRegKey, 
			i, 
			&ValueName[0],
			&ActualValueNameLength, NULL, &Type,
			EDIDData,
			&EDIDSize);

		if (RetValue != ERROR_SUCCESS || (FCString::Strcmp(ValueName, TEXT("EDID")) != 0))
		{
			continue;
		}

		// EDID data format documented here:
		// http://en.wikipedia.org/wiki/EDID

		int DetailTimingDescriptorStartIndex = 54;
		OutWidth = ((EDIDData[DetailTimingDescriptorStartIndex+4] >> 4) << 8) | EDIDData[DetailTimingDescriptorStartIndex+2];
		OutHeight = ((EDIDData[DetailTimingDescriptorStartIndex+7] >> 4) << 8) | EDIDData[DetailTimingDescriptorStartIndex+5];

		const int32 HorizontalSizeOffset = 21;
		const int32 VerticalSizeOffset = 22;
		const float CmToInch = 0.393701f;

		if (EDIDData[HorizontalSizeOffset] > 0 && EDIDData[VerticalSizeOffset] > 0)
		{
			float PhysicalWidth = CmToInch * (float)EDIDData[HorizontalSizeOffset];
			float PhysicalHeight = CmToInch * (float)EDIDData[VerticalSizeOffset];

			int32 HDpi = FMath::TruncToInt((float)OutWidth / PhysicalWidth);
			int32 VDpi = FMath::TruncToInt((float)OutHeight / PhysicalHeight);

			OutDPI = (HDpi + VDpi) / 2;
		}
		else
		{
			OutDPI = 0;
		}

		return true; // valid EDID found
	}

	return false; // EDID not found
}

inline bool GetSizeForDevID(const FString& TargetDevID, int32& Width, int32& Height, int32& DPI)
{
	static const GUID ClassMonitorGuid = {0x4d36e96e, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18}};

	HDEVINFO DevInfo = SetupDiGetClassDevsEx(
		&ClassMonitorGuid, //class GUID
		NULL,
		NULL,
		DIGCF_PRESENT,
		NULL,
		NULL,
		NULL);

	if (NULL == DevInfo)
	{
		return false;
	}

	bool bRes = false;

	for (ULONG MonitorIndex = 0; ERROR_NO_MORE_ITEMS != GetLastError(); ++MonitorIndex)
	{ 
		SP_DEVINFO_DATA DevInfoData;
		ZeroMemory(&DevInfoData, sizeof(DevInfoData));
		DevInfoData.cbSize = sizeof(DevInfoData);

		if (SetupDiEnumDeviceInfo(DevInfo, MonitorIndex, &DevInfoData) == TRUE)
		{
			TCHAR Buffer[MAX_DEVICE_ID_LEN];
			if (CM_Get_Device_ID(DevInfoData.DevInst, Buffer, MAX_DEVICE_ID_LEN, 0) == CR_SUCCESS)
			{
				FString DevID(Buffer);
				DevID.MidInline(8, DevID.Find(TEXT("\\"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 9) - 8, EAllowShrinking::No);
				if (DevID == TargetDevID)
				{
					HKEY hDevRegKey = SetupDiOpenDevRegKey(DevInfo, &DevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

					if (hDevRegKey && hDevRegKey != INVALID_HANDLE_VALUE)
					{
						bRes = GetMonitorSizeFromEDID(hDevRegKey, Width, Height, DPI);
						RegCloseKey(hDevRegKey);
						break;
					}
				}
			}
		}
	}

	if (SetupDiDestroyDeviceInfoList(DevInfo) == FALSE)
	{
		bRes = false;
	}

	return bRes;
}

int32 SOneInchSquare::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	TSharedPtr<SWindow> WindowOwner = nullptr;
	TSharedPtr<SWidget> Parent = GetParentWidget();
	while (Parent && Parent != SNullWidget::NullWidget)
	{
		if (Parent->Advanced_IsWindow())
		{
			WindowOwner = StaticCastSharedPtr<SWindow>(Parent);
			break;
		}
		Parent = Parent->GetParentWidget();
	}

	if (WindowOwner)
	{
		HMONITOR Monitor = MonitorFromWindow((HWND)WindowOwner->GetNativeWindow()->GetOSWindowHandle(), MONITOR_DEFAULTTONEAREST);
		if (Monitor != LatestDPIComputedMonitor)
		{
			LatestDPIComputedMonitor = Monitor;

			FDisplayMetrics Metrics;
			FDisplayMetrics::RebuildDisplayMetrics(Metrics);
			MONITORINFOEX MonitorInfo;
			MonitorInfo.cbSize = sizeof(MonitorInfo);
			GetMonitorInfo(LatestDPIComputedMonitor, &MonitorInfo);

			DISPLAY_DEVICE DisplayDevice;
			DisplayDevice.cb = sizeof(DisplayDevice);
			EnumDisplayDevices(MonitorInfo.szDevice, 0, &DisplayDevice, 0);
			
			FString TargetDevId = DisplayDevice.DeviceID;
			TargetDevId = TargetDevId.Mid (8, TargetDevId.Find (TEXT("\\"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 9) - 8);
			int32 OutWidth, OutHeight, OutDPI;
			GetSizeForDevID(TargetDevId, OutWidth, OutHeight, OutDPI);
			DPI = OutDPI;
		}

		FSlateLayoutTransform Transform(1.0f, AllottedGeometry.GetAbsolutePosition());
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			FGeometry::MakeRoot(FVector2D(DPI, DPI), Transform).ToPaintGeometry(),
			FCoreStyle::Get().GetDefaultBrush());
	}
	
	return LayerId;
}

TSharedRef<SWidget> UOneInchSquare::RebuildWidget()
{
    return SNew(SOneInchSquare);
}
