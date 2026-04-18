#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	Locked      UMETA(DisplayName = "잠김"),
	Available   UMETA(DisplayName = "수락 가능"),
	Active      UMETA(DisplayName = "진행 중"),
	Completed   UMETA(DisplayName = "완료"),
	Failed      UMETA(DisplayName = "실패")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
	KillEnemy       UMETA(DisplayName = "적 처치"),
	ReachLocation   UMETA(DisplayName = "위치 도달"),
	CollectItem     UMETA(DisplayName = "아이템 수집"),
	DefeatBoss      UMETA(DisplayName = "보스 처치"),
	Survive         UMETA(DisplayName = "생존"),
	Interact        UMETA(DisplayName = "오브젝트 상호작용")
};

USTRUCT(BlueprintType)
struct FQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Objective")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Objective")
	EObjectiveType Type = EObjectiveType::KillEnemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Objective")
	FName TargetID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Objective")
	int32 RequiredCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Objective")
	bool bCompleted = false;

	bool IsComplete() const { return CurrentCount >= RequiredCount; }
};

USTRUCT(BlueprintType)
struct FQuestData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	FName QuestID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	FText Title;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	EQuestStatus Status = EQuestStatus::Locked;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	TArray<FQuestObjective> Objectives;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest|Rewards")
	int32 SpiritPointReward = 500;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest|Rewards")
	TArray<FName> UnlockTransformations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Quest")
	TArray<FName> PrerequisiteQuestIDs;

	bool AllObjectivesComplete() const
	{
		for (const FQuestObjective& Obj : Objectives)
		{
			if (!Obj.bCompleted) return false;
		}
		return Objectives.Num() > 0;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted,    FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted,  FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, FName, ObjectiveID);

UCLASS(BlueprintType, Blueprintable)
class WUKONGGAME_API UQuestManager : public UObject
{
	GENERATED_BODY()

public:
	UQuestManager();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveUpdated OnObjectiveUpdated;

	UFUNCTION(BlueprintCallable, Category = "Quests")
	bool StartQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	bool CompleteQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	bool FailQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void UnlockQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void NotifyObjectiveProgress(EObjectiveType Type, FName TargetID, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Quests")
	EQuestStatus GetQuestStatus(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quests")
	bool IsQuestActive(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quests")
	bool IsQuestCompleted(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quests")
	TArray<FQuestData> GetActiveQuests() const;

	UFUNCTION(BlueprintCallable, Category = "Quests")
	void RegisterQuest(FQuestData QuestData);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quests")
	TArray<FQuestData> AllQuests;

private:
	FQuestData* FindQuest(FName QuestID);
	const FQuestData* FindQuestConst(FName QuestID) const;
	bool ArePrerequisitesMet(const FQuestData& Quest) const;
	void GrantQuestRewards(const FQuestData& Quest);
};
