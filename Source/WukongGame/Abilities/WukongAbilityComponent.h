#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WukongAbilityComponent.generated.h"

UENUM(BlueprintType)
enum class ETransformationType : uint8
{
	Normal      UMETA(DisplayName = "일반"),
	Giant       UMETA(DisplayName = "거대 변신 (법천상지)"),
	Tiny        UMETA(DisplayName = "소형 변신"),
	CloudSurf   UMETA(DisplayName = "근두운"),
	Disguise    UMETA(DisplayName = "변신 (72변)")
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	CloudSurf       UMETA(DisplayName = "근두운"),
	GiantForm       UMETA(DisplayName = "거대 변신"),
	TinyForm        UMETA(DisplayName = "소형 변신"),
	HairClone       UMETA(DisplayName = "분신술"),
	Immobilize      UMETA(DisplayName = "정신 (停神)"),
	StoneForm       UMETA(DisplayName = "석화 변신")
};

USTRUCT(BlueprintType)
struct FTransformationData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) ETransformationType Type = ETransformationType::Normal;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float ScaleMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float AttackMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float DefenseMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float SpeedMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float ManaCost = 30.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float Duration = 20.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) bool bUnlocked = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTransformationChanged,
	ETransformationType, NewForm, ETransformationType, OldForm);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityUsed, EAbilityType, Ability);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WUKONGGAME_API UWukongAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWukongAbilityComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTransformationChanged OnTransformationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAbilityUsed OnAbilityUsed;

	// --- 변신 ---
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool ActivateTransformation(ETransformationType Type);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RevertToNormal();

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void CycleTransformation();

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void UnlockTransformation(ETransformationType Type);

	// --- 근두운 ---
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ToggleCloudSurf();

	// --- 상태 ---
	UFUNCTION(BlueprintPure, Category = "Abilities")
	ETransformationType GetCurrentForm() const { return CurrentForm; }

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool IsTransformed() const { return CurrentForm != ETransformationType::Normal; }

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool IsCloudSurfing() const { return bIsCloudSurfing; }

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool IsAbilityOnCooldown(EAbilityType Ability) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FTransformationData> Transformations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	float CloudSurfManaCostPerSecond = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	float CloudSurfSpeed = 1200.f;

private:
	ETransformationType CurrentForm = ETransformationType::Normal;
	bool bIsCloudSurfing = false;
	float TransformTimer = 0.f;
	float CurrentTransformDuration = 0.f;
	int32 CurrentTransformIndex = 0;

	TMap<EAbilityType, float> CooldownTimers;

	void ApplyTransformation(const FTransformationData& Data);
	void RemoveTransformation();
	void UpdateCloudSurf(float DeltaTime);
	FTransformationData* FindTransformData(ETransformationType Type);
};
