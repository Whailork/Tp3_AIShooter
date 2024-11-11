// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAIController.h"

#include "AICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TP3Shoot/TP3ShootCharacter.h"

AShooterAIController::AShooterAIController(const FObjectInitializer& ObjectInitializer)
{
    // Create BT Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree Component"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard Component"));

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    SightConfig->PeripheralVisionAngleDegrees = 45.0f;
    SightConfig->SightRadius = 10.0f;
    SightConfig->SetMaxAge(6.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 2.0f;
    SightConfig->LoseSightRadius = 12.0f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComponent->ConfigureSense(*SightConfig);
    PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());

}

void AShooterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // Cast your Character/Pawn to get access to the attributes
    if (auto AICharactere = Cast<AAICharacter>(InPawn))
    {

        if (PerceptionComponent)
        {
            PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnTargetPerceptionUpdated);
            PerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &AShooterAIController::OnTargetPerceptionForgotten);
        }

        // Check if the assets has been selected in the editor
        if (AICharactere->TreeAsset) {

            BlackboardComponent->InitializeBlackboard(*AICharactere->TreeAsset->BlackboardAsset);
			
            // Run the BT
            BehaviorTreeComponent->StartTree(*AICharactere->TreeAsset);

            /*
            AICharactere->SetPacmanReference();
            
            // RENTRE
            if (AICharactere->PacManReference) {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Coucou1"));
            }

            // RENTRE
            if (BehaviorTreeComponent) {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Coucou2"));
            }

            // NE RENTRE PAS!?
            if (BehaviorTreeComponent->GetBlackboardComponent()) {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Coucou3"));
            }

            if (AICharactere->PacManReference && BehaviorTreeComponent && BehaviorTreeComponent->GetBlackboardComponent())
            {

                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Coucou"));

                // Set PacMan in the Blackboard
                BehaviorTreeComponent->GetBlackboardComponent()->SetValueAsVector("TargetLocation", AICharactere->PacManReference->GetActorLocation());

                FVector position = BehaviorTreeComponent->GetBlackboardComponent()->GetValueAsVector("TargetLocation");
            }
            */
        }
    }
}

void AShooterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    
    if (auto AiTarget = Cast<AAICharacter>(Actor))
    {       
        int TeamId = Cast <AAICharacter>(GetPawn())->TeamId;

        if (!AiTarget || AiTarget->TeamId == TeamId || AiTarget->isDead) return;

        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            // if stimulus is sight, check if stimulus is sensed
            if (Stimulus.WasSuccessfullySensed())
            {
                // Set can see player to true
                BlackboardComponent->SetValueAsBool("CanSeeEnemy", true);
                // Set target actor
                BlackboardComponent->SetValueAsObject("Enemy", AiTarget);
            }
            else
            {
                BlackboardComponent->SetValueAsBool("CanSeeEnemy", false);
                BlackboardComponent->SetValueAsObject("Enemy", nullptr);
                BlackboardComponent->SetValueAsVector("LastKnownLocation", AiTarget->GetActorLocation());
            }
        }
        
    }

    else if (auto PlayerTarget = Cast<ATP3ShootCharacter>(Actor))
    {
        
        int TeamId = Cast <AAICharacter>(GetPawn())->TeamId;

        if (!PlayerTarget || PlayerTarget->TeamId == TeamId) return;

        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            // if stimulus is sight, check if stimulus is sensed
            if (Stimulus.WasSuccessfullySensed())
            {
                // Set can see player to true
                BlackboardComponent->SetValueAsBool("CanSeeEnemy", true);
                // Set target actor
                BlackboardComponent->SetValueAsObject("Enemy", PlayerTarget);
            }
            else
            {
                BlackboardComponent->SetValueAsBool("CanSeeEnemy", false);
                BlackboardComponent->SetValueAsObject("Enemy", nullptr);
                BlackboardComponent->SetValueAsVector("LastKnownLocation", PlayerTarget->GetActorLocation());
            }
        }
        
    }
    
}

void AShooterAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
    ResetTarget();
    ResetLastKnownLocation();
}

void AShooterAIController::ResetTarget()
{
    BlackboardComponent->SetValueAsObject("Enemy", nullptr);
}

void AShooterAIController::ResetLastKnownLocation()
{
    BlackboardComponent->ClearValue("LastKnownLocation");
}

void AShooterAIController::ResetHealthLost()
{
    BlackboardComponent->SetValueAsBool("IsUnderFire", false);
    BlackboardComponent->SetValueAsObject("Shooter", nullptr);
    HealthLostTimerHandle.Invalidate();
}

void AShooterAIController::OnHealthLost()
{
    if(BlackboardComponent->GetValueAsObject("Shooter") == nullptr)
    {
        BlackboardComponent->SetValueAsBool("IsUnderFire", true);
        BlackboardComponent->SetValueAsObject("Shooter", Cast<AAICharacter>(GetPawn())->ShooterActor);
        GetWorld()->GetTimerManager().SetTimer(HealthLostTimerHandle, this, &AShooterAIController::ResetHealthLost, 5.0f, false);
    
    }
    
}

void AShooterAIController::OnDeath()
{
    auto AICharactere = Cast<AAICharacter>(GetPawn());
    BlackboardComponent->InitializeBlackboard(*AICharactere->TreeAsset->BlackboardAsset);
    BlackboardComponent->SetValueAsBool("IsDead", true);
}

void AShooterAIController::OnRespawn()
{
    BlackboardComponent->SetValueAsBool("IsDead", false);
}
