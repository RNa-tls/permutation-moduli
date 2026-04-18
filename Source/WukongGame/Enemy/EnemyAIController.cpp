#include "Enemy/EnemyAIController.h"
#include "Character/WukongCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree"));
	BlackboardComp   = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*PerceptionComp);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	SetupPerception();
}

void AEnemyAIController::SetupPerception()
{
	UAISenseConfig_Sight* SightCfg = NewObject<UAISenseConfig_Sight>(this);
	SightCfg->SightRadius = SightRadius;
	SightCfg->LoseSightRadius = SightRadius * 1.2f;
	SightCfg->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
	SightCfg->SetMaxAge(5.f);
	SightCfg->DetectionByAffiliation.bDetectEnemies = true;
	SightCfg->DetectionByAffiliation.bDetectNeutrals = false;
	SightCfg->DetectionByAffiliation.bDetectFriendlies = false;
	PerceptionComp->ConfigureSense(*SightCfg);

	UAISenseConfig_Hearing* HearingCfg = NewObject<UAISenseConfig_Hearing>(this);
	HearingCfg->HearingRange = HearingRange;
	HearingCfg->DetectionByAffiliation.bDetectEnemies = true;
	PerceptionComp->ConfigureSense(*HearingCfg);

	PerceptionComp->SetDominantSense(SightCfg->GetSenseImplementation());
	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	OwnerEnemy = Cast<AEnemyCharacter>(InPawn);

	if (OwnerEnemy && OwnerEnemy->BehaviorTree)
	{
		UseBlackboard(OwnerEnemy->BehaviorTree->BlackboardAsset, BlackboardComp);
		RunBehaviorTree(OwnerEnemy->BehaviorTree);
	}
}

void AEnemyAIController::OnUnPossess()
{
	if (BehaviorTreeComp) BehaviorTreeComp->StopTree();
	OwnerEnemy = nullptr;
	Super::OnUnPossess();
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BBUpdateTimer += DeltaTime;
	if (BBUpdateTimer >= BBUpdateRate)
	{
		BBUpdateTimer = 0.f;
		UpdateBlackboard();
	}
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* Actor : UpdatedActors)
	{
		AWukongCharacter* Player = Cast<AWukongCharacter>(Actor);
		if (!Player) continue;

		FActorPerceptionBlueprintInfo Info;
		PerceptionComp->GetActorsPerception(Actor, Info);

		bool bDetected = false;
		for (const FAIStimulus& Stim : Info.LastSensedStimuli)
		{
			if (Stim.WasSuccessfullySensed()) { bDetected = true; break; }
		}

		if (bDetected)
		{
			SetTarget(Player);
			if (OwnerEnemy) OwnerEnemy->SetEnemyState(EEnemyState::Chasing);
		}
	}
}

void AEnemyAIController::SetTarget(AActor* NewTarget)
{
	if (BlackboardComp) BlackboardComp->SetValueAsObject(BBKeys::TargetActor, NewTarget);
}

void AEnemyAIController::ClearTarget()
{
	if (BlackboardComp) BlackboardComp->ClearValue(BBKeys::TargetActor);
	if (OwnerEnemy) OwnerEnemy->SetEnemyState(EEnemyState::Patrolling);
}

AActor* AEnemyAIController::GetCurrentTarget() const
{
	if (!BlackboardComp) return nullptr;
	return Cast<AActor>(BlackboardComp->GetValueAsObject(BBKeys::TargetActor));
}

void AEnemyAIController::UpdateBlackboard()
{
	if (!BlackboardComp || !OwnerEnemy) return;

	AActor* Target = GetCurrentTarget();
	if (Target)
	{
		float Dist = FVector::Dist(OwnerEnemy->GetActorLocation(), Target->GetActorLocation());
		BlackboardComp->SetValueAsFloat(BBKeys::DistToTarget, Dist);
		BlackboardComp->SetValueAsBool(BBKeys::bCanAttack, Dist <= OwnerEnemy->AttackRadius);
	}
	BlackboardComp->SetValueAsBool(BBKeys::bIsStaggered,
		OwnerEnemy->GetEnemyState() == EEnemyState::Staggered);
}
