#include "Stats/ExperienceComponent.h"
#include "Math/UnrealMathUtility.h"

UExperienceComponent::UExperienceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExperienceComponent::AddSpiritPoints(int32 Amount)
{
	if (Amount <= 0 || CurrentLevel >= MaxLevel) return;

	TotalSpiritPoints += Amount;
	CurrentExp += Amount;
	OnExpGained.Broadcast(Amount, TotalSpiritPoints);
	CheckLevelUp();
}

void UExperienceComponent::CheckLevelUp()
{
	while (CurrentLevel < MaxLevel)
	{
		int32 Required = GetExpToNextLevel();
		if (CurrentExp < Required) break;

		int32 OldLevel = CurrentLevel;
		CurrentExp -= Required;
		CurrentLevel++;
		OnLevelUp.Broadcast(CurrentLevel, OldLevel);
	}
}

int32 UExperienceComponent::GetExpToNextLevel() const
{
	return CalculateExpForLevel(CurrentLevel);
}

float UExperienceComponent::GetExpPercent() const
{
	int32 Required = GetExpToNextLevel();
	if (Required <= 0) return 1.f;
	return FMath::Clamp((float)CurrentExp / (float)Required, 0.f, 1.f);
}

int32 UExperienceComponent::CalculateExpForLevel(int32 Level) const
{
	return FMath::RoundToInt(BaseExpPerLevel * FMath::Pow(ExpScalingFactor, Level - 1));
}
