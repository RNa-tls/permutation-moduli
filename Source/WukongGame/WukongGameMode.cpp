#include "WukongGameMode.h"
#include "Character/WukongCharacter.h"
#include "WukongGameInstance.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AWukongGameMode::AWukongGameMode()
{
	DefaultPawnClass = AWukongCharacter::StaticClass();
}

void AWukongGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AWukongGameMode::HandlePlayerDeath(AWukongCharacter* DeadPlayer)
{
	if (!DeadPlayer) return;

	DeathCount++;
	OnPlayerDied.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("GameMode: Player died (death #%d)."), DeathCount);

	if (MaxDeaths > 0 && DeathCount >= MaxDeaths)
	{
		// 게임 오버
		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
		return;
	}

	// 리스폰 타이머
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("ExecuteRespawn"), DeadPlayer);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, Delegate, RespawnDelay, false);
}

void AWukongGameMode::ExecuteRespawn(AWukongCharacter* Player)
{
	if (!Player) return;

	RespawnPlayer(Player);
}

void AWukongGameMode::RespawnPlayer(AWukongCharacter* Player)
{
	if (!Player) return;

	// 체크포인트 위치로 이동
	UWukongGameInstance* GI = GetWukongGameInstance();
	FTransform SpawnTransform = FTransform::Identity;

	if (GI && !GI->PlayerProgress.LastCheckpointLevel.IsEmpty())
	{
		SpawnTransform = GI->PlayerProgress.LastCheckpointTransform;
	}
	else
	{
		// PlayerStart 탐색
		AActor* Start = FindPlayerStart(nullptr);
		if (Start) SpawnTransform = Start->GetActorTransform();
	}

	Player->SetActorTransform(SpawnTransform);
	Player->SetWukongState(EWukongState::Idle);

	// HP 부분 회복 (리스폰 시 30%)
	if (UCharacterStatsComponent* Stats = Player->StatsComponent)
	{
		float HealAmount = Stats->BaseStats.MaxHP * 0.3f;
		Stats->Heal(HealAmount);
	}

	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Player->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
	{
		Player->EnableInput(PC);
	}

	OnPlayerRespawned.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("GameMode: Player respawned at checkpoint."));
}

void AWukongGameMode::NotifyBossDefeated(FName BossID)
{
	OnBossDefeatedInLevel.Broadcast(BossID);

	if (UWukongGameInstance* GI = GetWukongGameInstance())
	{
		GI->NotifyBossDefeated(BossID);
	}
}

UWukongGameInstance* AWukongGameMode::GetWukongGameInstance() const
{
	return Cast<UWukongGameInstance>(GetGameInstance());
}
