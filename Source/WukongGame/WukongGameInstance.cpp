#include "WukongGameInstance.h"
#include "GameSystems/WukongSaveGame.h"
#include "GameSystems/QuestManager.h"
#include "Kismet/GameplayStatics.h"

UWukongGameInstance::UWukongGameInstance()
{
}

void UWukongGameInstance::Init()
{
	Super::Init();

	QuestManager = NewObject<UQuestManager>(this, UQuestManager::StaticClass());

	LoadGame();
}

void UWukongGameInstance::AddSpiritPoints(int32 Amount)
{
	PlayerProgress.SpiritPoints += Amount;
	OnSpiritPointsChanged.Broadcast(PlayerProgress.SpiritPoints);
}

bool UWukongGameInstance::SpendSpiritPoints(int32 Amount)
{
	if (PlayerProgress.SpiritPoints < Amount) return false;
	PlayerProgress.SpiritPoints -= Amount;
	OnSpiritPointsChanged.Broadcast(PlayerProgress.SpiritPoints);
	return true;
}

void UWukongGameInstance::UnlockTransformation(FName TransformationID)
{
	if (!PlayerProgress.UnlockedTransformations.Contains(TransformationID))
	{
		PlayerProgress.UnlockedTransformations.Add(TransformationID);
		UE_LOG(LogTemp, Log, TEXT("GameInstance: Transformation [%s] unlocked."), *TransformationID.ToString());
	}
}

bool UWukongGameInstance::IsTransformationUnlocked(FName TransformationID) const
{
	return PlayerProgress.UnlockedTransformations.Contains(TransformationID);
}

void UWukongGameInstance::NotifyBossDefeated(FName BossID)
{
	if (!PlayerProgress.DefeatedBosses.Contains(BossID))
	{
		PlayerProgress.DefeatedBosses.Add(BossID);
		OnBossDefeated.Broadcast(BossID);

		if (QuestManager)
		{
			QuestManager->NotifyObjectiveProgress(EObjectiveType::DefeatBoss, BossID, 1);
		}

		UE_LOG(LogTemp, Log, TEXT("GameInstance: Boss [%s] defeated."), *BossID.ToString());
	}
}

bool UWukongGameInstance::IsBossDefeated(FName BossID) const
{
	return PlayerProgress.DefeatedBosses.Contains(BossID);
}

void UWukongGameInstance::SaveCheckpoint(const FString& LevelName, const FTransform& PlayerTransform)
{
	PlayerProgress.LastCheckpointLevel = LevelName;
	PlayerProgress.LastCheckpointTransform = PlayerTransform;
	SaveGame();
}

void UWukongGameInstance::SaveGame()
{
	UWukongSaveGame* SaveData = Cast<UWukongSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UWukongSaveGame::StaticClass()));

	if (!SaveData) return;

	SaveData->SavedProgress = PlayerProgress;
	SaveData->SaveTimestamp = FDateTime::Now();

	UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, SaveUserIndex);
	UE_LOG(LogTemp, Log, TEXT("GameInstance: Game saved to slot [%s]."), *SaveSlotName);
}

bool UWukongGameInstance::LoadGame()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex)) return false;

	UWukongSaveGame* SaveData = Cast<UWukongSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));

	if (!SaveData) return false;

	PlayerProgress = SaveData->SavedProgress;
	UE_LOG(LogTemp, Log, TEXT("GameInstance: Game loaded from slot [%s]."), *SaveSlotName);
	return true;
}
