#include "Abilities/WukongAbilityComponent.h"
#include "Stats/CharacterStatsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UWukongAbilityComponent::UWukongAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 변신 목록 초기화
	FTransformationData Normal; Normal.Type = ETransformationType::Normal; Normal.bUnlocked = true;
	FTransformationData Giant;  Giant.Type = ETransformationType::Giant;
		Giant.ScaleMultiplier = 3.f; Giant.AttackMultiplier = 2.5f;
		Giant.DefenseMultiplier = 2.f; Giant.SpeedMultiplier = 0.7f;
		Giant.ManaCost = 50.f; Giant.Duration = 15.f;
	FTransformationData Tiny;   Tiny.Type = ETransformationType::Tiny;
		Tiny.ScaleMultiplier = 0.15f; Tiny.SpeedMultiplier = 1.8f;
		Tiny.ManaCost = 20.f; Tiny.Duration = 30.f;

	Transformations = { Normal, Giant, Tiny };
}

void UWukongAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWukongAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 변신 지속시간 체크
	if (IsTransformed())
	{
		TransformTimer -= DeltaTime;
		if (TransformTimer <= 0.f) RevertToNormal();
	}

	// 근두운 마나 소모
	UpdateCloudSurf(DeltaTime);

	// 쿨다운 감소
	for (auto& Pair : CooldownTimers)
	{
		Pair.Value = FMath::Max(0.f, Pair.Value - DeltaTime);
	}
}

bool UWukongAbilityComponent::ActivateTransformation(ETransformationType Type)
{
	FTransformationData* Data = FindTransformData(Type);
	if (!Data || !Data->bUnlocked) return false;
	if (Type == ETransformationType::Normal) { RevertToNormal(); return true; }

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats || !Stats->ConsumeMana(Data->ManaCost)) return false;

	ETransformationType OldForm = CurrentForm;
	if (IsTransformed()) RemoveTransformation();

	ApplyTransformation(*Data);
	CurrentForm = Type;
	TransformTimer = Data->Duration;
	CurrentTransformDuration = Data->Duration;

	OnTransformationChanged.Broadcast(CurrentForm, OldForm);
	return true;
}

void UWukongAbilityComponent::RevertToNormal()
{
	if (CurrentForm == ETransformationType::Normal) return;
	ETransformationType OldForm = CurrentForm;
	RemoveTransformation();
	CurrentForm = ETransformationType::Normal;
	OnTransformationChanged.Broadcast(ETransformationType::Normal, OldForm);
}

void UWukongAbilityComponent::CycleTransformation()
{
	TArray<ETransformationType> Unlocked;
	for (const FTransformationData& T : Transformations)
	{
		if (T.bUnlocked && T.Type != ETransformationType::Normal)
			Unlocked.Add(T.Type);
	}
	if (Unlocked.Num() == 0) return;

	if (IsTransformed()) { RevertToNormal(); return; }

	ActivateTransformation(Unlocked[CurrentTransformIndex % Unlocked.Num()]);
	CurrentTransformIndex++;
}

void UWukongAbilityComponent::UnlockTransformation(ETransformationType Type)
{
	FTransformationData* Data = FindTransformData(Type);
	if (Data) Data->bUnlocked = true;
}

void UWukongAbilityComponent::ToggleCloudSurf()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	bIsCloudSurfing = !bIsCloudSurfing;

	if (bIsCloudSurfing)
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Char->GetCharacterMovement()->MaxFlySpeed = CloudSurfSpeed;
		OnAbilityUsed.Broadcast(EAbilityType::CloudSurf);
	}
	else
	{
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

bool UWukongAbilityComponent::IsAbilityOnCooldown(EAbilityType Ability) const
{
	const float* Timer = CooldownTimers.Find(Ability);
	return Timer && *Timer > 0.f;
}

void UWukongAbilityComponent::ApplyTransformation(const FTransformationData& Data)
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char) Char->SetActorScale3D(FVector(Data.ScaleMultiplier));

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (Stats) Stats->ApplyStatMultiplier(Data.AttackMultiplier, Data.DefenseMultiplier, Data.SpeedMultiplier);

	if (UCharacterMovementComponent* Move = Cast<ACharacter>(GetOwner())->GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 600.f * Data.SpeedMultiplier;
	}
}

void UWukongAbilityComponent::RemoveTransformation()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char) Char->SetActorScale3D(FVector(1.f));

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (Stats) Stats->ResetStatMultipliers();

	if (UCharacterMovementComponent* Move = Cast<ACharacter>(GetOwner())->GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 600.f;
	}
}

void UWukongAbilityComponent::UpdateCloudSurf(float DeltaTime)
{
	if (!bIsCloudSurfing) return;

	UCharacterStatsComponent* Stats = GetOwner()->FindComponentByClass<UCharacterStatsComponent>();
	if (!Stats || !Stats->ConsumeMana(CloudSurfManaCostPerSecond * DeltaTime))
	{
		ToggleCloudSurf();
	}
}

FTransformationData* UWukongAbilityComponent::FindTransformData(ETransformationType Type)
{
	for (FTransformationData& T : Transformations)
	{
		if (T.Type == Type) return &T;
	}
	return nullptr;
}
