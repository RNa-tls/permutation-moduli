#include "Interaction/ContextualInteractionComponent.h"
#include "Enemy/BossCharacter.h"
#include "Enemy/BossAIController.h"
#include "Character/WukongCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UContextualInteractionComponent::UContextualInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UContextualInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	FindPlayerAndBoss();
	SetupDefaultTriggers();
}

void UContextualInteractionComponent::FindPlayerAndBoss()
{
	OwnerBoss = Cast<ABossCharacter>(GetOwner());
	CachedPlayer = Cast<AWukongCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

void UContextualInteractionComponent::SetupDefaultTriggers()
{
	// --- 포션 사용 → 강탈 ---
	FInteractionTrigger StealPotion;
	StealPotion.TriggerAction  = EPlayerAction::UsePotion;
	StealPotion.BossReaction   = EBossReaction::StealPotion;
	StealPotion.DetectionRadius = 700.f;
	StealPotion.ReactionChance  = 0.85f;
	StealPotion.Cooldown        = 12.f;
	Triggers.Add(StealPotion);

	// --- 인벤토리 열기 → 강탈 기회 ---
	FInteractionTrigger InvOpen;
	InvOpen.TriggerAction   = EPlayerAction::OpenInventory;
	InvOpen.BossReaction    = EBossReaction::StealPotion;
	InvOpen.DetectionRadius = 700.f;
	InvOpen.ReactionChance  = 0.5f;
	InvOpen.Cooldown        = 15.f;
	Triggers.Add(InvOpen);

	// --- 여의봉 탑승 → 봉 잡고 흔들기 ---
	FInteractionTrigger GrabStaff;
	GrabStaff.TriggerAction   = EPlayerAction::MountStaff;
	GrabStaff.BossReaction    = EBossReaction::GrabStaff;
	GrabStaff.DetectionRadius = 1200.f;
	GrabStaff.ReactionChance  = 0.9f;
	GrabStaff.Cooldown        = 20.f;
	Triggers.Add(GrabStaff);

	// --- 여의봉 확장 → 봉 잡기 ---
	FInteractionTrigger ExtendStaff;
	ExtendStaff.TriggerAction   = EPlayerAction::ExtendStaff;
	ExtendStaff.BossReaction    = EBossReaction::GrabStaff;
	ExtendStaff.DetectionRadius = 1000.f;
	ExtendStaff.ReactionChance  = 0.6f;
	ExtendStaff.Cooldown        = 15.f;
	Triggers.Add(ExtendStaff);

	// --- 회피 → 발 밟기 카운터 ---
	FInteractionTrigger CounterDodge;
	CounterDodge.TriggerAction   = EPlayerAction::Dodging;
	CounterDodge.BossReaction    = EBossReaction::CounterDodge;
	CounterDodge.DetectionRadius = 350.f;
	CounterDodge.ReactionChance  = 0.35f;
	CounterDodge.Cooldown        = 6.f;
	Triggers.Add(CounterDodge);

	// --- 패리 시도 → 강공격으로 패리 부수기 ---
	FInteractionTrigger BreakParry;
	BreakParry.TriggerAction   = EPlayerAction::Parrying;
	BreakParry.BossReaction    = EBossReaction::BreakParry;
	BreakParry.DetectionRadius = 300.f;
	BreakParry.ReactionChance  = 0.4f;
	BreakParry.Cooldown        = 8.f;
	Triggers.Add(BreakParry);

	// --- 변신 중 → 기 흡수로 방해 ---
	FInteractionTrigger AntiTransform;
	AntiTransform.TriggerAction   = EPlayerAction::Transforming;
	AntiTransform.BossReaction    = EBossReaction::AntiTransform;
	AntiTransform.DetectionRadius = 900.f;
	AntiTransform.ReactionChance  = 0.7f;
	AntiTransform.Cooldown        = 25.f;
	Triggers.Add(AntiTransform);

	// --- 근두운 탑승 → 격추 ---
	FInteractionTrigger CloudAntiAir;
	CloudAntiAir.TriggerAction   = EPlayerAction::CloudSurfing;
	CloudAntiAir.BossReaction    = EBossReaction::CloudAntiAir;
	CloudAntiAir.DetectionRadius = 2000.f;
	CloudAntiAir.ReactionChance  = 0.65f;
	CloudAntiAir.Cooldown        = 18.f;
	Triggers.Add(CloudAntiAir);
}

void UContextualInteractionComponent::EvaluateTriggers(EPlayerAction PlayerAction)
{
	if (bIsReacting || !OwnerBoss || !OwnerBoss->IsAlive()) return;

	float Now = GetWorld()->GetTimeSeconds();
	float Dist = DistanceToPlayer();

	for (FInteractionTrigger& Trigger : Triggers)
	{
		if (Trigger.TriggerAction != PlayerAction) continue;
		if (Trigger.IsOnCooldown(Now)) continue;
		if (Dist > Trigger.DetectionRadius) continue;
		if (!RollChance(Trigger.ReactionChance)) continue;

		Trigger.LastReactionTime = Now;
		ExecuteReaction(Trigger.BossReaction);
		break; // 한 번에 하나만 반응
	}
}

void UContextualInteractionComponent::ExecuteReaction(EBossReaction Reaction)
{
	bIsReacting = true;

	// BossAIController에 반응 위임
	if (ABossAIController* BAI = Cast<ABossAIController>(OwnerBoss->GetController()))
	{
		switch (Reaction)
		{
		case EBossReaction::StealPotion:
			BAI->ReactStealPotion();
			OnInteractionTriggered.Broadcast(EPlayerAction::UsePotion, Reaction);
			break;

		case EBossReaction::GrabStaff:
			BAI->ReactGrabStaff();
			OnInteractionTriggered.Broadcast(EPlayerAction::ExtendStaff, Reaction);
			break;

		case EBossReaction::CounterDodge:
			BAI->ReactCounterDodge();
			OnInteractionTriggered.Broadcast(EPlayerAction::Dodging, Reaction);
			break;

		case EBossReaction::BreakParry:
			BAI->ReactBreakParry();
			OnInteractionTriggered.Broadcast(EPlayerAction::Parrying, Reaction);
			break;

		case EBossReaction::AntiTransform:
			// 변신 방해: 마나 흡수 (BossOwner가 직접 처리)
			if (CachedPlayer && CachedPlayer->StatsComponent)
			{
				CachedPlayer->StatsComponent->ConsumeMana(
					CachedPlayer->StatsComponent->BaseStats.MaxMana * 0.5f);
				UE_LOG(LogTemp, Log, TEXT("ContextualInteraction: Anti-transform — drained player mana!"));
			}
			OnInteractionTriggered.Broadcast(EPlayerAction::Transforming, Reaction);
			break;

		case EBossReaction::CloudAntiAir:
			// 근두운 격추: 큰 충격을 위쪽에서 아래로 가해 추락
			if (CachedPlayer)
			{
				CachedPlayer->LaunchCharacter(FVector(0.f, 0.f, -3000.f), false, true);
				UE_LOG(LogTemp, Log, TEXT("ContextualInteraction: Cloud surf knocked down!"));
			}
			OnInteractionTriggered.Broadcast(EPlayerAction::CloudSurfing, Reaction);
			break;

		default:
			break;
		}
	}

	bIsReacting = false;
}

void UContextualInteractionComponent::RegisterTrigger(FInteractionTrigger Trigger)
{
	Triggers.Add(Trigger);
}

void UContextualInteractionComponent::ClearTriggers()
{
	Triggers.Empty();
}

float UContextualInteractionComponent::DistanceToPlayer() const
{
	if (!OwnerBoss || !CachedPlayer) return MAX_FLT;
	return FVector::Dist(OwnerBoss->GetActorLocation(), CachedPlayer->GetActorLocation());
}

bool UContextualInteractionComponent::RollChance(float Chance) const
{
	return FMath::FRand() <= Chance;
}
