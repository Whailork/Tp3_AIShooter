// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAIController.h"

#include "AICharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AShooterAIController::AShooterAIController(const FObjectInitializer& ObjectInitializer)
{
    // Create BT Component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree Component"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard Component"));
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // Cast your Character/Pawn to get access to the attributes
    if (auto AICharactere = Cast<AAICharacter>(InPawn))
    {

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
