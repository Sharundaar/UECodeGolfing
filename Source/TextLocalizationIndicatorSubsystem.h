#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "TextLocalizationIndicatorSubsystem.generated.h"

UCLASS()
class UTextLocalizationIndicatorSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	FDelegateHandle DebugDrawDelegate;
};