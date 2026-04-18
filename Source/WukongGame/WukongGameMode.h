#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WukongGameMode.generated.h"

class AWukongCharacter;
class UWukongGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerRespawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDefeatedInLevel, FName, BossID);

UCLASS()
class WUKONGGAME_API AWukongGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWukongGameMode();

	virtual void BeginPlay() override;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRespawned OnPlayerRespawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossDefeatedInLevel OnBossDefeatedInLevel;

	// --- 플레이어 관리 ---
	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void HandlePlayerDeath(AWukongCharacter* DeadPlayer);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void RespawnPlayer(AWukongCharacter* Player);

	// --- 보스 처치 알림 ---
	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void NotifyBossDefeated(FName BossID);

	// --- 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	float RespawnDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Respawn")
	int32 MaxDeaths = -1; // -1 = 무제한

protected:
	UWukongGameInstance* GetWukongGameInstance() const;

private:
	int32 DeathCount = 0;

	FTimerHandle RespawnTimerHandle;

	void ExecuteRespawn(AWukongCharacter* Player);
};
