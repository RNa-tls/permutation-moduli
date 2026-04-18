#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaffWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaffExtended, float, ExtendedLength, FVector, TipLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaffRetracted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaffGrabbed, AActor*, Grabber);

UCLASS()
class WUKONGGAME_API AStaffWeapon : public AActor
{
	GENERATED_BODY()

public:
	AStaffWeapon();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Delegates (보스 AI가 구독) ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaffExtended OnStaffExtended;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaffRetracted OnStaffRetracted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaffGrabbed OnStaffGrabbed;

	// --- 여의봉 확장/축소 ---
	UFUNCTION(BlueprintCallable, Category = "Staff")
	void ExtendStaff(float TargetLength);

	UFUNCTION(BlueprintCallable, Category = "Staff")
	void RetractStaff();

	UFUNCTION(BlueprintCallable, Category = "Staff")
	void MaxExtend();

	// 보스가 여의봉을 잡을 때 호출
	UFUNCTION(BlueprintCallable, Category = "Staff")
	void OnBossGrab(AActor* BossActor);

	// 보스가 여의봉을 흔들어 플레이어를 추락시킬 때
	UFUNCTION(BlueprintCallable, Category = "Staff")
	void ShakeOffRider(float ImpulseForce);

	UFUNCTION(BlueprintPure, Category = "Staff")
	bool IsExtended() const { return bIsExtended; }

	UFUNCTION(BlueprintPure, Category = "Staff")
	float GetCurrentLength() const { return CurrentLength; }

	UFUNCTION(BlueprintPure, Category = "Staff")
	FVector GetTipLocation() const;

	// --- Config ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Staff")
	float DefaultLength = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Staff")
	float MaxLength = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Staff")
	float ExtendSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Staff")
	float RetractSpeed = 800.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaffMesh;

private:
	bool bIsExtended = false;
	bool bIsExtending = false;
	bool bIsRetracting = false;
	float CurrentLength;
	float TargetExtendLength;

	void UpdateStaffScale();
};
