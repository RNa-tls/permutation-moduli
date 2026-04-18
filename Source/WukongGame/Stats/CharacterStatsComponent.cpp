#include "Stats/CharacterStatsComponent.h"
#include "Math/UnrealMathUtility.h"

UCharacterStatsComponent::UCharacterStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = BaseStats.MaxHP;
	CurrentStamina = BaseStats.MaxStamina;
	CurrentMana = BaseStats.MaxMana;
}

void UCharacterStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RegenTick(DeltaTime);
}

void UCharacterStatsComponent::RegenTick(float DeltaTime)
{
	// Stamina regen (delayed after last use)
	StaminaRegenTimer += DeltaTime;
	if (StaminaRegenTimer >= BaseStats.StaminaRegenDelay && CurrentStamina < BaseStats.MaxStamina)
	{
		float NewStamina = FMath::Min(BaseStats.MaxStamina, CurrentStamina + BaseStats.StaminaRegenRate * DeltaTime);
		if (NewStamina != CurrentStamina)
		{
			CurrentStamina = NewStamina;
			OnStaminaChanged.Broadcast(CurrentStamina, BaseStats.MaxStamina);
		}
	}

	// Mana regen (always)
	if (CurrentMana < BaseStats.MaxMana)
	{
		float NewMana = FMath::Min(BaseStats.MaxMana, CurrentMana + BaseStats.ManaRegenRate * DeltaTime);
		if (NewMana != CurrentMana)
		{
			CurrentMana = NewMana;
			OnManaChanged.Broadcast(CurrentMana, BaseStats.MaxMana);
		}
	}
}

float UCharacterStatsComponent::TakeDamage(float RawDamage, bool bIsParried)
{
	if (!IsAlive()) return 0.f;

	float FinalDamage = CalculateDamage(RawDamage, bIsParried);
	CurrentHP = FMath::Max(0.f, CurrentHP - FinalDamage);
	OnHealthChanged.Broadcast(CurrentHP, BaseStats.MaxHP);

	if (CurrentHP <= 0.f)
	{
		OnCharacterDeath.Broadcast();
	}

	return FinalDamage;
}

float UCharacterStatsComponent::CalculateDamage(float RawDamage, bool bIsParried) const
{
	if (bIsParried) return 0.f;

	float Reduced = FMath::Max(1.f, RawDamage - GetDefense() * 0.5f);
	return Reduced;
}

void UCharacterStatsComponent::Heal(float Amount)
{
	if (!IsAlive()) return;
	CurrentHP = FMath::Min(BaseStats.MaxHP, CurrentHP + Amount);
	OnHealthChanged.Broadcast(CurrentHP, BaseStats.MaxHP);
}

float UCharacterStatsComponent::GetHealthPercent() const
{
	if (BaseStats.MaxHP <= 0.f) return 0.f;
	return CurrentHP / BaseStats.MaxHP;
}

bool UCharacterStatsComponent::ConsumeStamina(float Amount)
{
	if (CurrentStamina < Amount) return false;
	CurrentStamina -= Amount;
	StaminaRegenTimer = 0.f;
	OnStaminaChanged.Broadcast(CurrentStamina, BaseStats.MaxStamina);
	return true;
}

bool UCharacterStatsComponent::ConsumeMana(float Amount)
{
	if (CurrentMana < Amount) return false;
	CurrentMana -= Amount;
	OnManaChanged.Broadcast(CurrentMana, BaseStats.MaxMana);
	return true;
}

void UCharacterStatsComponent::RestoreMana(float Amount)
{
	CurrentMana = FMath::Min(BaseStats.MaxMana, CurrentMana + Amount);
	OnManaChanged.Broadcast(CurrentMana, BaseStats.MaxMana);
}

void UCharacterStatsComponent::ApplyStatMultiplier(float AttackMult, float DefenseMult, float SpeedMult)
{
	AttackMultiplier = AttackMult;
	DefenseMultiplier = DefenseMult;
	SpeedMultiplier = SpeedMult;
}

void UCharacterStatsComponent::ResetStatMultipliers()
{
	AttackMultiplier = 1.f;
	DefenseMultiplier = 1.f;
	SpeedMultiplier = 1.f;
}
