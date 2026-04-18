#include "Enemy/BossCharacter.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 보스 기본 스탯 (일반 적보다 훨씬 강함)
	StatsComponent->BaseStats.MaxHP = 3000.f;
	AttackDamage = 100.f;
	SpiritPointReward = 2000;

	// 기본 3페이즈 설정
	FBossPhase Phase1; Phase1.HealthThreshold = 1.f;   Phase1.bActivated = true;
	FBossPhase Phase2; Phase2.HealthThreshold = 0.6f;  Phase2.AttackDamageMultiplier = 1.4f;
		Phase2.SpeedMultiplier = 1.2f; Phase2.bUnlocksSpecialAttack = true;
	FBossPhase Phase3; Phase3.HealthThreshold = 0.25f; Phase3.AttackDamageMultiplier = 2.f;
		Phase3.SpeedMultiplier = 1.5f; Phase3.bBerserkMode = true; Phase3.bUnlocksSpecialAttack = true;

	Phases = { Phase1, Phase2, Phase3 };
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();
	OriginalAttackDamage = AttackDamage;
	if (GetCharacterMovement())
		OriginalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;

	if (Phases.Num() > 0)
	{
		Phases[0].bActivated = true;
		CurrentPhaseIndex = 0;
	}
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckPhaseTransition();
}

bool ABossCharacter::IsInBerserkMode() const
{
	return Phases.IsValidIndex(CurrentPhaseIndex) && Phases[CurrentPhaseIndex].bBerserkMode;
}

void ABossCharacter::CheckPhaseTransition()
{
	if (!IsAlive() || !StatsComponent) return;

	float HP = StatsComponent->GetHealthPercent();
	for (int32 i = CurrentPhaseIndex + 1; i < Phases.Num(); ++i)
	{
		if (!Phases[i].bActivated && HP <= Phases[i].HealthThreshold)
		{
			ActivatePhase(i);
			return;
		}
	}
}

void ABossCharacter::ActivatePhase(int32 PhaseIndex)
{
	if (!Phases.IsValidIndex(PhaseIndex)) return;

	CurrentPhaseIndex = PhaseIndex;
	Phases[PhaseIndex].bActivated = true;
	ApplyPhaseModifiers(Phases[PhaseIndex]);
	OnBossPhaseChanged.Broadcast(PhaseIndex);

	if (Phases[PhaseIndex].bUnlocksSpecialAttack)
		OnSpecialAttackReady.Broadcast();

	// 카메라 흔들기 (HitResult 없이 플레이어 컨트롤러에 직접)
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ClientStartCameraShake(nullptr);
	}
}

void ABossCharacter::ApplyPhaseModifiers(const FBossPhase& Phase)
{
	AttackDamage = OriginalAttackDamage * Phase.AttackDamageMultiplier;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
		Move->MaxWalkSpeed = OriginalMoveSpeed * Phase.SpeedMultiplier;
}

void ABossCharacter::PerformSpecialAttack(AActor* Target)
{
	if (!IsAlive() || !Target) return;

	float Now = GetWorld()->GetTimeSeconds();
	if ((Now - LastSpecialAttackTime) < SpecialAttackCooldown) return;
	LastSpecialAttackTime = Now;

	if (IsInBerserkMode())
	{
		PerformAreaSweep();
	}
	else
	{
		static bool bFlip = false;
		bFlip = !bFlip;
		if (bFlip) PerformGroundSlam();
		else       PerformRangedProjectile(Target);
	}
}

void ABossCharacter::PerformGroundSlam()
{
	TArray<AActor*> Ignored;
	Ignored.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(), GroundSlamDamage,
		GetActorLocation(), GroundSlamRadius, nullptr, Ignored, this, GetController(), true);
}

void ABossCharacter::PerformRangedProjectile(AActor* Target)
{
	if (!Target) return;
	UGameplayStatics::ApplyDamage(Target, RangedDamage, GetController(), this, nullptr);
}

void ABossCharacter::PerformAreaSweep()
{
	TArray<AActor*> Ignored;
	Ignored.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(), AttackDamage * 1.5f,
		GetActorLocation(), 500.f, nullptr, Ignored, this, GetController(), false);
}

void ABossCharacter::Die()
{
	Super::Die();
	// 보스 사망 시 퀘스트 매니저에 알림은 GameMode를 통해 처리
}
