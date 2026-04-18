#include "Character/WukongPlayerController.h"
#include "Character/WukongCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AWukongPlayerController::AWukongPlayerController()
{
	bShowMouseCursor = false;
}

void AWukongPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidget) HUDWidget->AddToViewport();
	}
}

void AWukongPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AWukongPlayerController::ShowDeathScreen()
{
	if (DeathScreenClass && !DeathScreenWidget)
	{
		DeathScreenWidget = CreateWidget<UUserWidget>(this, DeathScreenClass);
		if (DeathScreenWidget) DeathScreenWidget->AddToViewport();
	}
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}

void AWukongPlayerController::HideDeathScreen()
{
	if (DeathScreenWidget)
	{
		DeathScreenWidget->RemoveFromParent();
		DeathScreenWidget = nullptr;
	}
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
}

void AWukongPlayerController::RespawnPlayer()
{
	HideDeathScreen();
	UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()), false);
}
