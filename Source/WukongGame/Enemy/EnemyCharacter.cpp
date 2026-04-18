#include "Enemy/EnemyCharacter.h"
#include "Stats/CharacterStatsComponent.h"
#include "Stats/ExperienceComponent.h"
#include "Character/WukongCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	StatsComponent = CreateDefaultSubobject<UCharacterStatsComponent>(TEXT("StatsComponent"));
	StatsComponent->BaseStats.MaxHP = 200.f;
	StatsComponent->BaseStats.AttackPower = 30.f;
	StatsComponent->BaseStats.Defense = 10.f;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (StatsComponent)
		StatsComponent->OnCharacterDeath.AddDynamic(this, &AEnemyCharacter::OnDeathCallback);
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!StatsComponent || !IsAlive()) return 0.f;
	return StatsComponent->TakeDamage(DamageAmount);
}

void AEnemyCharacter::SetEnemyState(EEnemyState NewState)
{
	if (EnemyState == NewState) return;
	EnemyState = NewState;
	OnEnemyStateChanged.Broadcast(NewState);
}

bool AEnemyCharacter::IsAlive() const
{
	return StatsComponent && StatsComponent->IsAlive();
}

void AEnemyCharacter::PerformAttack(AActor* Target)
{
	if (!Target || !IsAlive()) return;

	float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	if (Dist > AttackRadius * 1.5f) return;

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);
	SetEnemyState(EEnemyState::Attacking);
}

void AEnemyCharacter::Die()
{
	if (EnemyState == EEnemyState::Dead) return;
	SetEnemyState(EEnemyState::Dead);

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 플레이어에게 영혼 포인트 지급
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player)
	{
		UExperienceComponent* ExpComp = Player->FindComponentByClass<UExperienceComponent>();
		if (ExpComp) ExpComp->AddSpiritPoints(SpiritPointReward);
	}

	OnEnemyDied.Broadcast();

	// 3초 후 제거
	SetLifeSpan(3.f);
}

void AEnemyCharacter::OnDeathCallback()
{
	Die();
}
