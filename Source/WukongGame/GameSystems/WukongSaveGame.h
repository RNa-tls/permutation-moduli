#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WukongGameInstance.h"
#include "WukongSaveGame.generated.h"

UCLASS()
class WUKONGGAME_API UWukongSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UWukongSaveGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save")
	FPlayerProgressData SavedProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FString SaveVersion = TEXT("1.0.0");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	FDateTime SaveTimestamp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save|Meta")
	float TotalPlayTimeSeconds = 0.f;
};
