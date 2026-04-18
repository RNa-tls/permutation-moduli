#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterStatsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChanged, float, NewMana, float, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterDeath);

USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxHP = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxMana = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float AttackPower = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float Defense = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float CritChance = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float CritMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float StaminaRegenRate = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float ManaRegenRate = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float StaminaRegenDelay = 1.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UCharacterStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterStatsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnManaChanged OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterDeath OnCharacterDeath;

	// --- Health ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	float TakeDamage(float RawDamage, bool bIsParried = false);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void Heal(float Amount);

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool IsAlive() const { return CurrentHP > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetHealthPercent() const;

	// --- Stamina ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool HasStamina(float Amount) const { return CurrentStamina >= Amount; }

	// --- Mana ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool ConsumeMana(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RestoreMana(float Amount);

	// --- Stat Modifiers ---
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyStatMultiplier(float AttackMult, float DefenseMult, float SpeedMult);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ResetStatMultipliers();

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetAttackPower() const { return BaseStats.AttackPower * AttackMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetDefense() const { return BaseStats.Defense * DefenseMultiplier; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FCharacterStats BaseStats;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHP;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentStamina;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentMana;

private:
	float AttackMultiplier = 1.f;
	float DefenseMultiplier = 1.f;
	float SpeedMultiplier = 1.f;
	float StaminaRegenTimer = 0.f;

	void RegenTick(float DeltaTime);
	float CalculateDamage(float RawDamage, bool bIsParried) const;
};
