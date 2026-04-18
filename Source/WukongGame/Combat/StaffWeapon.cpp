#include "Combat/StaffWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AStaffWeapon::AStaffWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	StaffMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaffMesh"));
	RootComponent = StaffMesh;
	StaffMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CurrentLength = DefaultLength;
}

void AStaffWeapon::BeginPlay()
{
	Super::BeginPlay();
	CurrentLength = DefaultLength;
	UpdateStaffScale();
}

void AStaffWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsExtending)
	{
		CurrentLength = FMath::FInterpTo(CurrentLength, TargetExtendLength, DeltaTime, ExtendSpeed / 100.f);
		UpdateStaffScale();

		if (FMath::IsNearlyEqual(CurrentLength, TargetExtendLength, 5.f))
		{
			CurrentLength = TargetExtendLength;
			bIsExtended = true;
			bIsExtending = false;
			UpdateStaffScale();
			OnStaffExtended.Broadcast(CurrentLength, GetTipLocation());
		}
	}
	else if (bIsRetracting)
	{
		CurrentLength = FMath::FInterpTo(CurrentLength, DefaultLength, DeltaTime, RetractSpeed / 100.f);
		UpdateStaffScale();

		if (FMath::IsNearlyEqual(CurrentLength, DefaultLength, 5.f))
		{
			CurrentLength = DefaultLength;
			bIsExtended = false;
			bIsRetracting = false;
			UpdateStaffScale();
			OnStaffRetracted.Broadcast();
		}
	}
}

void AStaffWeapon::ExtendStaff(float TargetLength)
{
	TargetExtendLength = FMath::Clamp(TargetLength, DefaultLength, MaxLength);
	bIsExtending = true;
	bIsRetracting = false;
}

void AStaffWeapon::RetractStaff()
{
	bIsRetracting = true;
	bIsExtending = false;
}

void AStaffWeapon::MaxExtend()
{
	ExtendStaff(MaxLength);
}

void AStaffWeapon::OnBossGrab(AActor* BossActor)
{
	OnStaffGrabbed.Broadcast(BossActor);
}

void AStaffWeapon::ShakeOffRider(float ImpulseForce)
{
	// 봉 위에 올라탄 플레이어(Owner)를 밀쳐냄
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	ACharacter* Player = Cast<ACharacter>(OwnerActor);
	if (!Player) return;

	FVector LaunchDir = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 1.f).GetSafeNormal();
	Player->LaunchCharacter(LaunchDir * ImpulseForce, true, true);

	// 봉도 축소
	RetractStaff();
}

FVector AStaffWeapon::GetTipLocation() const
{
	return GetActorLocation() + GetActorUpVector() * CurrentLength;
}

void AStaffWeapon::UpdateStaffScale()
{
	if (!StaffMesh) return;
	float ScaleFactor = CurrentLength / DefaultLength;
	StaffMesh->SetRelativeScale3D(FVector(1.f, 1.f, ScaleFactor));
}
