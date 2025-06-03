// Copyright Sharundaar. All Rights Reserved.

#include "MinimalUserWidgetExtension.h"

#if WITH_EDITOR
#include "UMGEditorModule.h"
#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#endif

void UMinimalUserWidgetExtension::Initialize()
{
}

void UMinimalWidgetBlueprintGeneratedClassExtension::Initialize(UUserWidget* UserWidget)
{
	// Happens rigth before the user widget Initialize BP event, this is where we'd add the per user widget extension
	// or potentially do any manipulation on the user widget, you can whatever you want here
	UMinimalUserWidgetExtension* InstanceExtension = UserWidget->AddExtension<UMinimalUserWidgetExtension>();
	InstanceExtension->Color = Color;
	InstanceExtension->Name = Name;
}

void UMinimalWidgetBlueprintGeneratedClassExtension::PreConstruct(UUserWidget* UserWidget, bool IsDesignTime)
{
	if(UserWidget->IsDesignTime()) // Initialize isn't called in the designer, so call it manually here so we can do fun stuff in PreConstruct at design time
	{
		Initialize(UserWidget); // @NOTE: we might want to harden this code here and check that extension doesn't exists already, or it'll be added twice
	}
}

#if WITH_EDITORONLY_DATA
void UMinimalWidgetBlueprintExtension::HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class)
{
	check(CurrentContext);

	if (CurrentContext->bIsFullCompile)
	{
		UMinimalWidgetBlueprintGeneratedClassExtension* GeneratedBPExtension = NewObject<UMinimalWidgetBlueprintGeneratedClassExtension>(Class);
// @TODO: Copy or set any relevant property on the blueprint generated class, this will be accessible at runtime
		GeneratedBPExtension->Color = Color;
		GeneratedBPExtension->Name = Name;
		CurrentContext->AddExtension(Class, GeneratedBPExtension);
	}
}
#endif

#if WITH_EDITOR // @NOTE: There is no "generic" extension editor, so we have to build our own
namespace MinimalWidgetExtensionEditor
{
	class SMinimalWidgetExtensionEditor : public SCompoundWidget, public FNotifyHook
	{
	public:
		SLATE_BEGIN_ARGS(SMinimalWidgetExtensionEditor) {}
		SLATE_END_ARGS()

		SMinimalWidgetExtensionEditor() = default;

		TWeakObjectPtr<UMinimalWidgetBlueprintExtension> MinimalBlueprintExtension;

		void HandleExtensionAdded(UBlueprintExtension* BlueprintExtension)
		{
			// Cleanup and rebuild our content when the extension is added
			if (UMinimalWidgetBlueprintExtension* MinimalExtension = Cast<UMinimalWidgetBlueprintExtension>(BlueprintExtension))
			{
				UWidgetBlueprint* WidgetBlueprint = MinimalExtension->GetWidgetBlueprint();
				check(WidgetBlueprint);

				WidgetBlueprint->OnExtensionAdded.RemoveAll(this);
				MinimalBlueprintExtension = MinimalExtension;
				UpdateContent();
			}
		}

		void Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> Editor)
		{
			WeakWidgetEditor = Editor;

			UWidgetBlueprint* WidgetBlueprint = Editor->GetWidgetBlueprintObj();
			check(WidgetBlueprint);

			MinimalBlueprintExtension = UMinimalWidgetBlueprintExtension::GetExtension<UMinimalWidgetBlueprintExtension>(WidgetBlueprint);
			if (!MinimalBlueprintExtension.IsValid())
			{
				WidgetBlueprint->OnExtensionAdded.AddSP(this, &SMinimalWidgetExtensionEditor::HandleExtensionAdded);
			}
			UpdateContent();
		}

		// @NOTE: Here we'll build the actual editor window, which has a button to add the extension, and a details view to editor the extensions property
		// You can treat this whole code as a custom details panel for the extension object
		void UpdateContent()
		{
			// Details View ptr that's going to hold a reference to our extension
			if (!MinimalExtensionDetailsView)
			{
				FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

				FDetailsViewArgs DetailsViewArgs = {};
				DetailsViewArgs.bShowPropertyMatrixButton = false;
				DetailsViewArgs.bShowOptions = false;
				DetailsViewArgs.NotifyHook = this;
				DetailsViewArgs.bHideSelectionTip = true;
				DetailsViewArgs.bAllowSearch = false;

				MinimalExtensionDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
			}

			UMinimalWidgetBlueprintExtension* Extension = MinimalBlueprintExtension.Get();
			MinimalExtensionDetailsView->SetObject(Extension);

			ChildSlot[
				SNew(SOverlay)

				// "Add Minimal Blueprint Extension" Button in the middle, only visible if the extension hasn't been added yet 
				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Visibility_Lambda([this]()
					{
						return MinimalBlueprintExtension.IsValid() ? EVisibility::Collapsed : EVisibility::Visible;
					})
					.OnClicked_Lambda([this]()
					{
						if(const TSharedPtr<FWidgetBlueprintEditor> WidgetEditor = WeakWidgetEditor.Pin())
						{
							UWidgetBlueprint* WidgetBlueprint = WidgetEditor->GetWidgetBlueprintObj();
							checkf(WidgetBlueprint, TEXT("We assume the editor always has a valid widget blueprint object."));

							UMinimalWidgetBlueprintExtension::RequestExtension<UMinimalWidgetBlueprintExtension>(WidgetBlueprint); // This will trigger the OnExtensionAdded event
						}
						return FReply::Handled();
					})
					.Text(INVTEXT("Add Minimal Blueprint Extension"))
				]

				// The _actual_ extension editor, only visible if the extension has been added
				+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
				[
					SNew(SBox).Visibility_Lambda([this]()
					{
						return MinimalBlueprintExtension.IsValid() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
					})
					[
						MinimalExtensionDetailsView ? MinimalExtensionDetailsView.ToSharedRef() : SNullWidget::NullWidget
					]
				]
			];
		}

		TWeakPtr<FWidgetBlueprintEditor> WeakWidgetEditor;
		TSharedPtr<IDetailsView> MinimalExtensionDetailsView;
	};

	// @NOTE: Tab Summoner, this struct is responsible for spawning our editor tab, it's registered straight in the UMG BP editor
	struct FMinimalWidgetExtensionTabSummoner final : FWorkflowTabFactory
	{
		static const FName TabID;
		FMinimalWidgetExtensionTabSummoner(const TSharedPtr<FWidgetBlueprintEditor>& BlueprintEditor)
			: FWorkflowTabFactory(TabID, BlueprintEditor),
			  WeakBlueprintEditor(BlueprintEditor)
		{
			TabLabel = INVTEXT("Minimal Widget Extension");
			TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataLayer");

			bIsSingleton = true;
		}

		virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override
		{
			TSharedPtr<FWidgetBlueprintEditor> BlueprintEditorPtr = WeakBlueprintEditor.Pin();
			return SNew(SMinimalWidgetExtensionEditor, BlueprintEditorPtr).AddMetaData<FTagMetaData>(FTagMetaData(TabID));
		}

	protected:
		TWeakPtr<FWidgetBlueprintEditor> WeakBlueprintEditor;
	};

	const FName FMinimalWidgetExtensionTabSummoner::TabID = "MinimalWidgetExtensionEditorPanel";

	void HandleRegisterBlueprintEditorTab(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
	{
		if(ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::GraphMode
		|| ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::DesignerMode)
		{
			TabFactories.RegisterFactory(MakeShared<FMinimalWidgetExtensionTabSummoner>(ApplicationMode.GetBlueprintEditor()));
		}
	}

	FDelegateHandle RegisterBlueprintEditorTabDelegate;
}

void MinimalWidgetExtensionEditor::RegisterEditorTab()
{
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	RegisterBlueprintEditorTabDelegate = UMGEditorModule.OnRegisterTabsForEditor().AddStatic(&MinimalWidgetExtensionEditor::HandleRegisterBlueprintEditorTab);
}

void MinimalWidgetExtensionEditor::UnregisterEditorTab()
{
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
	UMGEditorModule.OnRegisterTabsForEditor().Remove(RegisterBlueprintEditorTabDelegate);
	RegisterBlueprintEditorTabDelegate.Reset();
}
#endif
