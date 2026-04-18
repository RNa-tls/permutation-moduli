#include "Combat/CombatComponent.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 5연 콤보 설정
	FComboAttack C1; C1.AnimationTag = "Attack_1"; C1.DamageMultiplier = 1.0f; C1.StaminaCost = 10.f;
	FComboAttack C2; C2.AnimationTag = "Attack_2"; C2.DamageMultiplier = 1.1f; C2.StaminaCost = 10.f;
	FComboAttack C3; C3.AnimationTag = "Attack_3"; C3.DamageMultiplier = 1.2f; C3.StaminaCost = 12.f;
	FComboAttack C4; C4.AnimationTag = "Attack_4"; C4.DamageMultiplier = 1.3f; C4.StaminaCost = 12.f;
	FComboAttack C5; C5.AnimationTag = "Attack_5"; C5.DamageMultiplier = 2.0f; C5.StaminaCost = 15.f; C5.bIsFinisher = true;
	ComboChain = { C1, C2, C3, C4, C5 };
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 경직 타이머
	if (CombatState == ECombatState::Staggered)
	{
		StaggerTimer -= DeltaTime;
		if (StaggerTimer <= 0.f) SetState(ECombatState::Idle);
	}

	// 패리 타이머
	if (CombatState == ECombatState::Parrying)
	{
		ParryTimer -= DeltaTime;
		if (ParryTimer <= 0.f) EndParry();
	}

	// 무기 히트 트레이스
	if (bWeaponTraceActive) PerformWeaponTrace();
}

void UCombatComponent::PerformAttack()
{
	if (CombatState == ECombatState::Staggered || CombatState == ECombatState::Dodging) return;

	if (CombatState == ECombatState::Attacking)
	{
		if (bComboWindowOpen) AdvanceCombo();
		else bAttackQueued = true;
		return;
	}

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats) return;

	const FComboAttack& Combo = ComboChain[CurrentComboIndex];
	if (!Stats->ConsumeStamina(Combo.StaminaCost)) return;

	SetState(ECombatState::Attacking);
	OnComboPerformed.Broadcast(CurrentComboIndex);
}

void UCombatComponent::AdvanceCombo()
{
	bComboWindowOpen = false;
	bAttackQueued = false;
	CurrentComboIndex = (CurrentComboIndex + 1) % ComboChain.Num();

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats) return;

	const FComboAttack& Combo = ComboChain[CurrentComboIndex];
	if (!Stats->ConsumeStamina(Combo.StaminaCost))
	{
		ResetCombo();
		return;
	}

	OnComboPerformed.Broadcast(CurrentComboIndex);
}

bool UCombatComponent::TryDodge(FVector Direction)
{
	if (CombatState == ECombatState::Staggered) return false;

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats || !Stats->ConsumeStamina(DodgeStaminaCost)) return false;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return false;

	SetState(ECombatState::Dodging);
	Owner->LaunchCharacter(Direction.GetSafeNormal() * DodgeImpulse, true, false);
	OnDodge.Broadcast();

	// 회피 후 자동 복귀 (애니메이션 노티파이로 처리 권장)
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
	{
		if (CombatState == ECombatState::Dodging) SetState(ECombatState::Idle);
	});

	return true;
}

void UCombatComponent::StartParry()
{
	if (CombatState != ECombatState::Idle) return;
	SetState(ECombatState::Parrying);
	ParryTimer = ParryWindowDuration;
}

void UCombatComponent::EndParry()
{
	if (CombatState == ECombatState::Parrying) SetState(ECombatState::Idle);
}

void UCombatComponent::OpenComboWindow()
{
	bComboWindowOpen = true;
	if (bAttackQueued)
	{
		bAttackQueued = false;
		AdvanceCombo();
	}
}

void UCombatComponent::CloseComboWindow()
{
	bComboWindowOpen = false;
}

void UCombatComponent::EnableWeaponTrace()
{
	bWeaponTraceActive = true;
	HitActorsThisSwing.Empty();
}

void UCombatComponent::DisableWeaponTrace()
{
	bWeaponTraceActive = false;
}

void UCombatComponent::OnAttackAnimEnd()
{
	if (!bComboWindowOpen) ResetCombo();
}

void UCombatComponent::ResetCombo()
{
	CurrentComboIndex = 0;
	bComboWindowOpen = false;
	bAttackQueued = false;
	SetState(ECombatState::Idle);
}

void UCombatComponent::ApplyStagger(float Duration)
{
	SetState(ECombatState::Staggered);
	StaggerTimer = Duration;
	DisableWeaponTrace();
}

void UCombatComponent::SetState(ECombatState NewState)
{
	CombatState = NewState;
}

void UCombatComponent::PerformWeaponTrace()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 소켓 기반 트레이스 (실제 구현시 WeaponActor의 소켓 좌표 사용)
	FVector Start = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 50.f;
	FVector End = Start + Owner->GetActorForwardVector() * 150.f;

	TArray<FHitResult> Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	GetWorld()->SweepMultiByChannel(Hits, Start, End,
		FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(40.f), Params);

	UCharacterStatsComponent* Stats = Owner->FindComponentByClass<UCharacterStatsComponent>();

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActorsThisSwing.Contains(HitActor)) continue;
		HitActorsThisSwing.Add(HitActor);

		float Damage = Stats ? Stats->GetAttackPower() * ComboChain[CurrentComboIndex].DamageMultiplier : 50.f;

		UCharacterStatsComponent* TargetStats = HitActor->FindComponentByClass<UCharacterStatsComponent>();
		if (TargetStats)
		{
			TargetStats->TakeDamage(Damage);
			OnHitDealt.Broadcast(HitActor, Damage);
		}
	}
}
