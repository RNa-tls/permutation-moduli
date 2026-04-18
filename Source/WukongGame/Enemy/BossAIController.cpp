#include "Enemy/BossAIController.h"
#include "Enemy/BossCharacter.h"
#include "Character/WukongCharacter.h"
#include "Combat/CombatComponent.h"
#include "Combat/StaffWeapon.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/WukongItem.h"
#include "Interaction/ContextualInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossAIController::ABossAIController()
{
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BossOwner = Cast<ABossCharacter>(InPawn);

	// 월드의 플레이어 캐릭터를 찾아 델리게이트 바인딩
	if (AWukongCharacter* Player = Cast<AWukongCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		TrackedPlayer = Player;
		BindToPlayer(Player);
		SetTarget(Player);
	}

	// ContextualInteractionComponent 가져오기
	if (BossOwner)
	{
		InteractionComp = BossOwner->FindComponentByClass<UContextualInteractionComponent>();
	}
}

void ABossAIController::OnUnPossess()
{
	if (TrackedPlayer)
	{
		UnbindFromPlayer(TrackedPlayer);
		TrackedPlayer = nullptr;
	}

	BossOwner = nullptr;
	Super::OnUnPossess();
}

void ABossAIController::BindToPlayer(AWukongCharacter* Player)
{
	if (!Player) return;

	// 인벤토리 이벤트 구독
	if (Player->InventoryComponent)
	{
		Player->InventoryComponent->OnItemUsed.AddDynamic(
			this, &ABossAIController::OnPlayerUsedItem);
		Player->InventoryComponent->OnInventoryOpened.AddDynamic(
			this, &ABossAIController::OnPlayerOpenedInventory);
	}

	// 여의봉 이벤트 구독
	if (Player->Staff)
	{
		Player->Staff->OnStaffExtended.AddDynamic(
			this, &ABossAIController::OnStaffExtended);
		Player->Staff->OnStaffRetracted.AddDynamic(
			this, &ABossAIController::OnStaffRetracted);
	}

	// 전투 이벤트 구독
	if (Player->CombatComponent)
	{
		Player->CombatComponent->OnDodge.AddDynamic(
			this, &ABossAIController::OnPlayerDodged);
		Player->CombatComponent->OnComboPerformed.AddDynamic(
			this, &ABossAIController::OnPlayerCombo);
	}
}

void ABossAIController::UnbindFromPlayer(AWukongCharacter* Player)
{
	if (!Player) return;

	if (Player->InventoryComponent)
	{
		Player->InventoryComponent->OnItemUsed.RemoveDynamic(
			this, &ABossAIController::OnPlayerUsedItem);
		Player->InventoryComponent->OnInventoryOpened.RemoveDynamic(
			this, &ABossAIController::OnPlayerOpenedInventory);
	}

	if (Player->Staff)
	{
		Player->Staff->OnStaffExtended.RemoveDynamic(
			this, &ABossAIController::OnStaffExtended);
		Player->Staff->OnStaffRetracted.RemoveDynamic(
			this, &ABossAIController::OnStaffRetracted);
	}

	if (Player->CombatComponent)
	{
		Player->CombatComponent->OnDodge.RemoveDynamic(
			this, &ABossAIController::OnPlayerDodged);
		Player->CombatComponent->OnComboPerformed.RemoveDynamic(
			this, &ABossAIController::OnPlayerCombo);
	}
}

// ── 플레이어 행동 감지 ──────────────────────────────────────────────────────

void ABossAIController::OnPlayerUsedItem(UWukongItem* Item, int32 RemainingCount)
{
	if (!Item || !CanReact()) return;

	// 포션류(HP 회복 아이템)이고 강탈 가능하면 반응
	if (Item->bBossCanSteal && Item->Effect == EItemEffect::HealHP
		&& DistanceToPlayer() <= StealPotionRange)
	{
		UE_LOG(LogTemp, Log, TEXT("BossAI: Player used potion — initiating steal!"));
		ReactStealPotion();
	}
}

void ABossAIController::OnPlayerOpenedInventory()
{
	if (!CanReact() || DistanceToPlayer() > StealPotionRange) return;

	// 인벤토리 열기도 강탈 기회로 활용 (확률 50%)
	if (FMath::RandBool())
	{
		UE_LOG(LogTemp, Log, TEXT("BossAI: Player opened inventory — initiating steal!"));
		ReactStealPotion();
	}
}

void ABossAIController::OnStaffExtended(float ExtendedLength, FVector TipLocation)
{
	if (!CanReact() || DistanceToPlayer() > GrabStaffRange) return;

	// 여의봉이 충분히 길 때만 잡기 (절반 이상 확장)
	if (BossOwner && TrackedPlayer && TrackedPlayer->Staff)
	{
		float MaxLen = TrackedPlayer->Staff->MaxLength;
		if (ExtendedLength >= MaxLen * 0.5f)
		{
			UE_LOG(LogTemp, Log, TEXT("BossAI: Staff extended — grabbing!"));
			ReactGrabStaff();
		}
	}
}

void ABossAIController::OnStaffRetracted()
{
	// 봉이 축소되면 잡기 행동 중단 (행동 트리에서 처리)
}

void ABossAIController::OnPlayerDodged()
{
	if (!CanReact() || DistanceToPlayer() > CounterDodgeRange) return;

	// 회피 카운터 (발 밟기) - 30% 확률
	if (FMath::FRand() < 0.3f)
	{
		UE_LOG(LogTemp, Log, TEXT("BossAI: Player dodged — counter stomp!"));
		ReactCounterDodge();
	}
}

void ABossAIController::OnPlayerCombo(int32 ComboIndex)
{
	// 5연속 콤보 피니셔 시 패리 부수기 반응
	if (ComboIndex >= 4 && CanReact())
	{
		UE_LOG(LogTemp, Log, TEXT("BossAI: Player finisher detected — break parry!"));
		ReactBreakParry();
	}
}

// ── 보스 반응 행동 ──────────────────────────────────────────────────────────

void ABossAIController::ReactStealPotion()
{
	if (!BossOwner || !TrackedPlayer || !TrackedPlayer->InventoryComponent) return;

	MarkReacted();

	// 인벤토리에서 포션 강탈
	UWukongItem* Stolen = TrackedPlayer->InventoryComponent->StealConsumableItem();
	if (Stolen && Stolen->BossHealOnSteal > 0.f)
	{
		// 보스 체력 회복
		if (UCharacterStatsComponent* Stats = BossOwner->FindComponentByClass<UCharacterStatsComponent>())
		{
			Stats->Heal(Stolen->BossHealOnSteal);
			UE_LOG(LogTemp, Log, TEXT("BossAI: Stole [%s], boss healed %.0f HP."),
				*Stolen->ItemName.ToString(), Stolen->BossHealOnSteal);
		}
	}

	// 보스가 플레이어 쪽으로 대시 (이동 시스템에 위임)
	if (GetPawn() && TrackedPlayer)
	{
		FVector DashDir = (TrackedPlayer->GetActorLocation() - BossOwner->GetActorLocation()).GetSafeNormal();
		BossOwner->LaunchCharacter(DashDir * 1200.f, true, false);
	}
}

void ABossAIController::ReactGrabStaff()
{
	if (!BossOwner || !TrackedPlayer || !TrackedPlayer->Staff) return;

	MarkReacted();

	AStaffWeapon* Staff = TrackedPlayer->Staff;

	// 여의봉을 잡고 흔들어 플레이어를 추락
	Staff->OnBossGrab(BossOwner);
	Staff->ShakeOffRider(2000.f);

	UE_LOG(LogTemp, Log, TEXT("BossAI: Grabbed staff and shook off rider!"));
}

void ABossAIController::ReactCounterDodge()
{
	if (!BossOwner || !TrackedPlayer) return;

	MarkReacted();

	// 즉각 짧은 발 밟기 공격
	UGameplayStatics::ApplyDamage(
		TrackedPlayer,
		BossOwner->AttackDamage * 0.5f,
		this,
		BossOwner,
		nullptr
	);

	// 플레이어 경직
	if (UCombatComponent* PlayerCombat = TrackedPlayer->CombatComponent)
	{
		PlayerCombat->ApplyStagger(0.8f);
	}
}

void ABossAIController::ReactBreakParry()
{
	if (!BossOwner || !TrackedPlayer) return;

	MarkReacted();

	// 강공격으로 패리 돌파
	UGameplayStatics::ApplyDamage(
		TrackedPlayer,
		BossOwner->AttackDamage * 2.0f,
		this,
		BossOwner,
		nullptr
	);
}

// ── 내부 헬퍼 ──────────────────────────────────────────────────────────────

bool ABossAIController::CanReact() const
{
	if (!BossOwner || !BossOwner->IsAlive()) return false;
	if (!TrackedPlayer) return false;

	float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastReactionTime) >= ReactionCooldown;
}

void ABossAIController::MarkReacted()
{
	LastReactionTime = GetWorld()->GetTimeSeconds();
}

float ABossAIController::DistanceToPlayer() const
{
	if (!BossOwner || !TrackedPlayer) return MAX_FLT;
	return FVector::Dist(BossOwner->GetActorLocation(), TrackedPlayer->GetActorLocation());
}
