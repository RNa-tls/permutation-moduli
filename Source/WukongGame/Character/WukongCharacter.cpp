#include "Character/WukongCharacter.h"
#include "Stats/CharacterStatsComponent.h"
#include "Stats/ExperienceComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/StaffWeapon.h"
#include "Abilities/WukongAbilityComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"

AWukongCharacter::AWukongCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 카메라 암
	USpringArmComponent* SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;

	UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	// 이동
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->JumpZVelocity = 600.f;

	// 컴포넌트
	StatsComponent = CreateDefaultSubobject<UCharacterStatsComponent>(TEXT("StatsComponent"));
	ExperienceComponent = CreateDefaultSubobject<UExperienceComponent>(TEXT("ExperienceComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	AbilityComponent = CreateDefaultSubobject<UWukongAbilityComponent>(TEXT("AbilityComponent"));
}

void AWukongCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Enhanced Input 등록
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 사망 이벤트 연결
	if (StatsComponent)
		StatsComponent->OnCharacterDeath.AddDynamic(this, &AWukongCharacter::OnDeath);

	SpawnStaff();
}

void AWukongCharacter::SpawnStaff()
{
	if (!StaffClass) return;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Staff = GetWorld()->SpawnActor<AStaffWeapon>(StaffClass, GetActorTransform(), Params);
	if (Staff)
	{
		Staff->AttachToComponent(GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale, TEXT("hand_rSocket"));
	}
}

void AWukongCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWukongCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWukongCharacter::Look);
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AWukongCharacter::Attack);
		EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AWukongCharacter::Dodge);
		EIC->BindAction(ParryAction, ETriggerEvent::Started, this, &AWukongCharacter::StartParry);
		EIC->BindAction(ParryAction, ETriggerEvent::Completed, this, &AWukongCharacter::StopParry);
		EIC->BindAction(ExtendStaffAction, ETriggerEvent::Started, this, &AWukongCharacter::ExtendStaff);
		EIC->BindAction(CloudSurfAction, ETriggerEvent::Started, this, &AWukongCharacter::ToggleCloudSurf);
		EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AWukongCharacter::ToggleInventory);
		EIC->BindAction(TransformAction, ETriggerEvent::Started, this, &AWukongCharacter::ActivateTransform);
	}
}

void AWukongCharacter::Move(const FInputActionValue& Value)
{
	if (WukongState == EWukongState::Dead) return;

	FVector2D MoveVec = Value.Get<FVector2D>();
	if (Controller)
	{
		FRotator Rot = Controller->GetControlRotation();
		FRotator YawRot(0.f, Rot.Yaw, 0.f);
		AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), MoveVec.Y);
		AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), MoveVec.X);
	}
}

void AWukongCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVec = Value.Get<FVector2D>();
	AddControllerYawInput(LookVec.X);
	AddControllerPitchInput(LookVec.Y);
}

void AWukongCharacter::Attack(const FInputActionValue& Value)
{
	if (WukongState == EWukongState::Dead) return;
	if (CombatComponent) CombatComponent->PerformAttack();
}

void AWukongCharacter::Dodge(const FInputActionValue& Value)
{
	if (WukongState == EWukongState::Dead) return;
	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero()) Dir = -GetActorForwardVector();
	if (CombatComponent) CombatComponent->TryDodge(Dir);
}

void AWukongCharacter::StartParry(const FInputActionValue& Value)
{
	if (WukongState == EWukongState::Dead) return;
	if (CombatComponent) CombatComponent->StartParry();
}

void AWukongCharacter::StopParry(const FInputActionValue& Value)
{
	if (CombatComponent) CombatComponent->EndParry();
}

void AWukongCharacter::ExtendStaff(const FInputActionValue& Value)
{
	if (!Staff || WukongState == EWukongState::Dead) return;
	if (Staff->IsExtended()) Staff->RetractStaff();
	else Staff->MaxExtend();
}

void AWukongCharacter::ToggleCloudSurf(const FInputActionValue& Value)
{
	if (!AbilityComponent || WukongState == EWukongState::Dead) return;
	AbilityComponent->ToggleCloudSurf();
}

void AWukongCharacter::ToggleInventory(const FInputActionValue& Value)
{
	if (!InventoryComponent) return;
	if (InventoryComponent->IsInventoryOpen()) InventoryComponent->CloseInventory();
	else InventoryComponent->OpenInventory();
}

void AWukongCharacter::ActivateTransform(const FInputActionValue& Value)
{
	if (!AbilityComponent || WukongState == EWukongState::Dead) return;
	AbilityComponent->CycleTransformation();
}

float AWukongCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!StatsComponent || WukongState == EWukongState::Dead) return 0.f;

	bool bIsParrying = CombatComponent && CombatComponent->IsParrying();
	float Applied = StatsComponent->TakeDamage(DamageAmount, bIsParrying);

	if (bIsParrying && Applied == 0.f)
	{
		CombatComponent->OnParrySuccess.Broadcast();
	}

	return Applied;
}

void AWukongCharacter::SetWukongState(EWukongState NewState)
{
	WukongState = NewState;
}

void AWukongCharacter::OnDeath()
{
	SetWukongState(EWukongState::Dead);
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 컨트롤러에 사망 알림
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}

void AWukongCharacter::OnCombatStateChanged()
{
}
