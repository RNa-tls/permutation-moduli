#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExperienceComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, OldLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpGained, int32, Amount, int32, TotalExp);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UExperienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExperienceComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnExpGained OnExpGained;

	UFUNCTION(BlueprintCallable, Category = "Experience")
	void AddSpiritPoints(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Experience")
	int32 GetLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Experience")
	int32 GetCurrentExp() const { return CurrentExp; }

	UFUNCTION(BlueprintPure, Category = "Experience")
	int32 GetExpToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Experience")
	float GetExpPercent() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
	int32 BaseExpPerLevel = 1000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
	float ExpScalingFactor = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
	int32 MaxLevel = 50;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	int32 CurrentExp = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	int32 TotalSpiritPoints = 0;

private:
	void CheckLevelUp();
	int32 CalculateExpForLevel(int32 Level) const;
};
