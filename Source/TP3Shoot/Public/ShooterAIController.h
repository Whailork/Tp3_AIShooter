// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"
UCLASS()
class TP3SHOOT_API AShooterAIController : public AAIController
{
	GENERATED_BODY()
	
	AShooterAIController(const FObjectInitializer& ObjectInitializer);
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(Transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent;
	UPROPERTY(Transient)
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UAISenseConfig_Sight* SightConfig;

};
