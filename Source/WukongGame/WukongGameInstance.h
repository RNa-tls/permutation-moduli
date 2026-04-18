#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WukongGameInstance.generated.h"

class UQuestManager;

USTRUCT(BlueprintType)
struct FPlayerProgressData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 Level = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 TotalExperience = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	int32 SpiritPoints = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	FString LastCheckpointLevel;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	FTransform LastCheckpointTransform;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	TArray<FName> UnlockedTransformations;

	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	TArray<FName> DefeatedBosses;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpiritPointsChanged, int32, NewAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDefeated, FName, BossID);

UCLASS()
class WUKONGGAME_API UWukongGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UWukongGameInstance();

	virtual void Init() override;

	// --- 진행 데이터 ---
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	FPlayerProgressData PlayerProgress;

	// --- 기령점 (Spirit Points) ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSpiritPointsChanged OnSpiritPointsChanged;

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void AddSpiritPoints(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	bool SpendSpiritPoints(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetSpiritPoints() const { return PlayerProgress.SpiritPoints; }

	// --- 변신 해금 ---
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void UnlockTransformation(FName TransformationID);

	UFUNCTION(BlueprintPure, Category = "Progress")
	bool IsTransformationUnlocked(FName TransformationID) const;

	// --- 보스 처치 기록 ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossDefeated OnBossDefeated;

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void NotifyBossDefeated(FName BossID);

	UFUNCTION(BlueprintPure, Category = "Progress")
	bool IsBossDefeated(FName BossID) const;

	// --- 체크포인트 ---
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void SaveCheckpoint(const FString& LevelName, const FTransform& PlayerTransform);

	// --- 저장/불러오기 ---
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame();

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame();

	// --- 퀘스트 ---
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	UQuestManager* QuestManager;

	// --- 설정 ---
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	FString SaveSlotName = TEXT("WukongSave_01");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Save")
	int32 SaveUserIndex = 0;
};
