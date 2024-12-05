// Copyright Sharundaar. All Rights Reserved.

#pragma once

#if WITH_EDITORONLY_DATA
#include "WidgetBlueprintExtension.h"
#endif

#include "Extensions/UserWidgetExtension.h"
#include "Extensions/WidgetBlueprintGeneratedClassExtension.h"

#include "MinimalUserWidgetExtension.generated.h"

// User Widget Extension, this object is stored on the widget instance
// This one will be unique per widget instance and is directly accessible with the GetExtension node in BP
// You'd store per-instance data in here
UCLASS()
class UMinimalUserWidgetExtension : public UUserWidgetExtension
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

protected:
// @TODO: Any properties you wish to hold in your user widget extension
// Here they are just random for demonstration, carried down from the generated class extension
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FColor Color;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString Name;

	friend class UMinimalWidgetBlueprintGeneratedClassExtension; // friend so the class extension can set our properties for simplicity
};

// Generated Class Extension, this object is stored in the UClass
// All of your widget instances will share the same class extension object so you'd store "global" data here
UCLASS()
class UMinimalWidgetBlueprintGeneratedClassExtension : public UWidgetBlueprintGeneratedClassExtension
{
	GENERATED_BODY()

public:
	virtual void Initialize(UUserWidget* UserWidget) override;
	virtual void PreConstruct(UUserWidget* UserWidget, bool IsDesignTime) override;

protected:
// @TODO: Any properties you wish to hold in your generated class extension
// Here they are just random for demonstration, carried down from the BP extension
	UPROPERTY()
	FColor Color;

	UPROPERTY()
	FString Name;

	friend class UMinimalWidgetBlueprintExtension; // friend so the BP extension can set our properties for simplicity
};

#if WITH_EDITORONLY_DATA
// Blueprint Extension, this object is attached to the UBlueprint object
// Only exists in editor, this disappears at runtime so store any runtime information in the Generated Class Extension
UCLASS()
class UMinimalWidgetBlueprintExtension : public UWidgetBlueprintExtension
{
	GENERATED_BODY()

protected:
	virtual void HandleBeginCompilation(FWidgetBlueprintCompilerContext& CreationContext) override { CurrentContext = &CreationContext; }
	virtual void HandleEndCompilation() override { CurrentContext = nullptr; }
	
	virtual void HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class) override;

private:
	FWidgetBlueprintCompilerContext* CurrentContext;


protected:
// @TODO: Any properties you wish to hold in your blueprint extension
// Here they are just random for demonstration
	UPROPERTY(EditAnywhere)
	FColor Color;

	UPROPERTY(EditAnywhere)
	FString Name;
};
#endif

#if WITH_EDITOR // @NOTE: There is no "generic" extension editor, so we have to build our own
namespace MinimalWidgetExtensionEditor
{
	// @TODO: Replace this with relevant module api
	// call in an editor module StartupModule and ShutdownModule functions typically 
	YOURMODULE_API void RegisterEditorTab();
	YOURMODULE_API void UnregisterEditorTab();
}
#endif