#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combat/CombatComponent.h"
#include "WukongCharacter.generated.h"

class UCharacterStatsComponent;
class UExperienceComponent;
class UInventoryComponent;
class UCombatComponent;
class UWukongAbilityComponent;
class AStaffWeapon;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EWukongState : uint8
{
	Idle          UMETA(DisplayName = "대기"),
	Running       UMETA(DisplayName = "달리기"),
	Attacking     UMETA(DisplayName = "공격"),
	Dodging       UMETA(DisplayName = "회피"),
	Parrying      UMETA(DisplayName = "패리"),
	Transformed   UMETA(DisplayName = "변신"),
	CloudSurfing  UMETA(DisplayName = "근두운"),
	Dead          UMETA(DisplayName = "사망")
};

UCLASS()
class WUKONGGAME_API AWukongCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWukongCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCharacterStatsComponent* StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExperienceComponent* ExperienceComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWukongAbilityComponent* AbilityComponent;

	// --- Staff ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AStaffWeapon> StaffClass;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AStaffWeapon* Staff;

	// --- State ---
	UFUNCTION(BlueprintPure, Category = "State")
	EWukongState GetWukongState() const { return WukongState; }

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetWukongState(EWukongState NewState);

	// --- Enhanced Input ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ParryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* ExtendStaffAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* CloudSurfAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* TransformAction;

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);
	void StartParry(const FInputActionValue& Value);
	void StopParry(const FInputActionValue& Value);
	void ExtendStaff(const FInputActionValue& Value);
	void ToggleCloudSurf(const FInputActionValue& Value);
	void ToggleInventory(const FInputActionValue& Value);
	void ActivateTransform(const FInputActionValue& Value);

private:
	EWukongState WukongState = EWukongState::Idle;

	UFUNCTION()
	void OnDeath();

	UFUNCTION()
	void OnCombatStateChanged();

	void SpawnStaff();
};
