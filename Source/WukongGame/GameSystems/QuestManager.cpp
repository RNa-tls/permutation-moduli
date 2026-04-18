#include "GameSystems/QuestManager.h"
#include "WukongGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
	// 기본 챕터 퀘스트: 여의봉 획득
	FQuestData Q1;
	Q1.QuestID    = FName("Quest_FindStaff");
	Q1.Title      = FText::FromString(TEXT("하늘의 지팡이"));
	Q1.Description= FText::FromString(TEXT("용궁에서 여의금고봉을 찾아라."));
	Q1.Status     = EQuestStatus::Available;
	Q1.SpiritPointReward = 0;

	FQuestObjective Obj1;
	Obj1.ObjectiveID  = FName("Obj_DefeatDragonGuard");
	Obj1.Description  = FText::FromString(TEXT("용궁 수호신을 물리쳐라"));
	Obj1.Type         = EObjectiveType::DefeatBoss;
	Obj1.TargetID     = FName("Boss_DragonGuard");
	Obj1.RequiredCount= 1;
	Q1.Objectives.Add(Obj1);

	AllQuests.Add(Q1);

	// 두 번째 퀘스트: 72변 습득
	FQuestData Q2;
	Q2.QuestID    = FName("Quest_Learn72");
	Q2.Title      = FText::FromString(TEXT("72변의 시작"));
	Q2.Description= FText::FromString(TEXT("삼청동자를 찾아 변신술을 배워라."));
	Q2.Status     = EQuestStatus::Locked;
	Q2.SpiritPointReward = 1000;
	Q2.PrerequisiteQuestIDs.Add(FName("Quest_FindStaff"));
	Q2.UnlockTransformations.Add(FName("Transform_Giant"));

	FQuestObjective Obj2;
	Obj2.ObjectiveID  = FName("Obj_FindSanQing");
	Obj2.Description  = FText::FromString(TEXT("삼청동자와 대화하라"));
	Obj2.Type         = EObjectiveType::Interact;
	Obj2.TargetID     = FName("NPC_SanQing");
	Obj2.RequiredCount= 1;
	Q2.Objectives.Add(Obj2);

	AllQuests.Add(Q2);
}

bool UQuestManager::StartQuest(FName QuestID)
{
	FQuestData* Quest = FindQuest(QuestID);
	if (!Quest || Quest->Status != EQuestStatus::Available) return false;
	if (!ArePrerequisitesMet(*Quest)) return false;

	Quest->Status = EQuestStatus::Active;
	OnQuestStarted.Broadcast(QuestID);
	UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest [%s] started."), *QuestID.ToString());
	return true;
}

bool UQuestManager::CompleteQuest(FName QuestID)
{
	FQuestData* Quest = FindQuest(QuestID);
	if (!Quest || Quest->Status != EQuestStatus::Active) return false;
	if (!Quest->AllObjectivesComplete()) return false;

	Quest->Status = EQuestStatus::Completed;
	GrantQuestRewards(*Quest);
	OnQuestCompleted.Broadcast(QuestID);

	// 선행 조건이 이 퀘스트인 퀘스트들을 Available로 전환
	for (FQuestData& OtherQuest : AllQuests)
	{
		if (OtherQuest.Status != EQuestStatus::Locked) continue;
		if (ArePrerequisitesMet(OtherQuest))
		{
			OtherQuest.Status = EQuestStatus::Available;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest [%s] COMPLETED."), *QuestID.ToString());
	return true;
}

bool UQuestManager::FailQuest(FName QuestID)
{
	FQuestData* Quest = FindQuest(QuestID);
	if (!Quest || Quest->Status != EQuestStatus::Active) return false;
	Quest->Status = EQuestStatus::Failed;
	return true;
}

void UQuestManager::UnlockQuest(FName QuestID)
{
	FQuestData* Quest = FindQuest(QuestID);
	if (Quest && Quest->Status == EQuestStatus::Locked)
	{
		Quest->Status = EQuestStatus::Available;
	}
}

void UQuestManager::NotifyObjectiveProgress(EObjectiveType Type, FName TargetID, int32 Amount)
{
	for (FQuestData& Quest : AllQuests)
	{
		if (Quest.Status != EQuestStatus::Active) continue;

		for (FQuestObjective& Obj : Quest.Objectives)
		{
			if (Obj.bCompleted || Obj.Type != Type || Obj.TargetID != TargetID) continue;

			Obj.CurrentCount = FMath::Min(Obj.RequiredCount, Obj.CurrentCount + Amount);
			if (Obj.CurrentCount >= Obj.RequiredCount)
			{
				Obj.bCompleted = true;
			}

			OnObjectiveUpdated.Broadcast(Quest.QuestID, Obj.ObjectiveID);

			if (Quest.AllObjectivesComplete())
			{
				CompleteQuest(Quest.QuestID);
			}
		}
	}
}

EQuestStatus UQuestManager::GetQuestStatus(FName QuestID) const
{
	const FQuestData* Quest = FindQuestConst(QuestID);
	return Quest ? Quest->Status : EQuestStatus::Locked;
}

bool UQuestManager::IsQuestActive(FName QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Active;
}

bool UQuestManager::IsQuestCompleted(FName QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Completed;
}

TArray<FQuestData> UQuestManager::GetActiveQuests() const
{
	TArray<FQuestData> Active;
	for (const FQuestData& Q : AllQuests)
	{
		if (Q.Status == EQuestStatus::Active) Active.Add(Q);
	}
	return Active;
}

void UQuestManager::RegisterQuest(FQuestData QuestData)
{
	for (const FQuestData& Q : AllQuests)
	{
		if (Q.QuestID == QuestData.QuestID) return;
	}
	AllQuests.Add(QuestData);
}

FQuestData* UQuestManager::FindQuest(FName QuestID)
{
	for (FQuestData& Q : AllQuests)
	{
		if (Q.QuestID == QuestID) return &Q;
	}
	return nullptr;
}

const FQuestData* UQuestManager::FindQuestConst(FName QuestID) const
{
	for (const FQuestData& Q : AllQuests)
	{
		if (Q.QuestID == QuestID) return &Q;
	}
	return nullptr;
}

bool UQuestManager::ArePrerequisitesMet(const FQuestData& Quest) const
{
	for (const FName& PrereqID : Quest.PrerequisiteQuestIDs)
	{
		if (!IsQuestCompleted(PrereqID)) return false;
	}
	return true;
}

void UQuestManager::GrantQuestRewards(const FQuestData& Quest)
{
	// GameInstance까지 Outer 체인을 타고 올라가 보상 지급
	UObject* Outer = GetOuter();
	while (Outer)
	{
		if (UWukongGameInstance* GI = Cast<UWukongGameInstance>(Outer))
		{
			GI->AddSpiritPoints(Quest.SpiritPointReward);
			for (const FName& TransID : Quest.UnlockTransformations)
			{
				GI->UnlockTransformation(TransID);
			}
			return;
		}
		Outer = Outer->GetOuter();
	}
}
