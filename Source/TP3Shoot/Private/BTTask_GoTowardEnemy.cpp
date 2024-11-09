// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_GoTowardEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include <ShooterAIController.h>
#include <AICharacter.h>
#include <TP3Shoot/TP3ShootCharacter.h>



EBTNodeResult::Type UBTTask_GoTowardEnemy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    /*
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    UObject* EnemyObject = BlackboardComp->GetValueAsObject("Enemy");
    if (EnemyObject != nullptr || EnemyObject->IsA<AActor>())
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to cast EnemyObject of type: %s"), *EnemyObject->GetClass()->GetName());
        // Get the target actor from the blackboard
        AActor* TargetActor = Cast<AActor>(EnemyObject);
        if (!TargetActor)
        {
            return EBTNodeResult::Failed;
        }

        // Check if the target is a valid type (AAIShooterCharacter or APlayerShooterCharacter)
        if (Cast<AAICharacter>(TargetActor) || Cast<ATP3ShootCharacter>(TargetActor))
        {
            float DistanceToTarget = FVector::Dist(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());

            // Move to the target only if the AI is farther than the stopping distance
            if (DistanceToTarget > StopDistance)
            {
                AIController->MoveToActor(TargetActor, StopDistance);
                return EBTNodeResult::InProgress; // Task continues until the movement is complete
            }
            else
            {
                return EBTNodeResult::Succeeded; // AI is already at the desired range
            }
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("EnemyObject is nullptr"));
    }
    */

    return EBTNodeResult::Failed;
}
