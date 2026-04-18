#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WukongPlayerController.generated.h"

UCLASS()
class WUKONGGAME_API AWukongPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWukongPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowDeathScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideDeathScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RespawnPlayer();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> DeathScreenClass;

private:
	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY()
	UUserWidget* DeathScreenWidget;
};
