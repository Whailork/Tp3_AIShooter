// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAIController.h"

#include "AICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AShooterAIController::AShooterAIController(const FObjectInitializer& ObjectInitializer)
{
    // Create BT Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree Component"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard Component"));

    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    SetPerceptionComponent(*PerceptionComponent);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

    SightConfig->PeripheralVisionAngleDegrees = 45.0f;
    SightConfig->SightRadius = 3000.0f;
    SightConfig->SetMaxAge(10.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 200.0f;
    SightConfig->LoseSightRadius = 3500;
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
    // Check if the perceived actor is valid and log the detection
    if (Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Warning, TEXT("Detected: %s"), *Actor->GetName());
    }
}
