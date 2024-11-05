// Fill out your copyright notice in the Description page of Project Settings.


#include "AICharacter.h"

#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	
	// Create SK_Gun
	SK_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	SK_Gun->SetupAttachment(GetMesh());
	// Set parent socket
	SK_Gun->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("GripPoint"));

	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	
}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = StartingHealth;
	spawnPoint = GetActorLocation();
	
}

int AAICharacter::getStartingHealth()
{
	return StartingHealth;
}

void AAICharacter::loseHealth(int amount)
{
	Health -= amount;
	if( Health <= 0)
	{
		auto rotation = GetActorTransform().GetRotation();
				
		GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &AAICharacter::Respawn, 3, false);
		GetMovementComponent()->StopMovementImmediately();
		GetMovementComponent()->StopActiveMovement();
		GetMovementComponent()->SetComponentTickEnabled(false);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		
	}
}

int AAICharacter::getHealth()
{
	return Health;
}

void AAICharacter::Respawn()
{
	TeleportTo(spawnPoint+FVector(0,0,100),FRotator(0,0,0));
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetRelativeLocation(FVector(0,0,-90));
	GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
	
	GetMovementComponent()->SetComponentTickEnabled(true);
	
	Health = StartingHealth;
	
	RespawnTimerHandle.Invalidate();
}

bool AAICharacter::isAlly()
{
	return ally;
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAICharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AAICharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AAICharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AAICharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AAICharacter::Aim()
{
	IsAiming = true;
}

void AAICharacter::StopAiming()
{
	IsAiming = false;
}

void AAICharacter::Fire(AActor* Target)
{
	FVector Start, LineTraceEnd, ForwardVector;
	FHitResult HitResult;


	// Get muzzle location
	Start = SK_Gun->GetSocketLocation("MuzzleFlash");

	// Get Rotation Forward Vector
	ForwardVector = FollowCamera->GetForwardVector();

	// Get End Point
	LineTraceEnd = Start + (ForwardVector * 10000);
	

	
	bool bSuccess = Controller->GetWorld()->LineTraceSingleByChannel(HitResult,Start,LineTraceEnd,ECollisionChannel::ECC_WorldDynamic);
	FireParticle(Start,HitResult,SK_Gun->GetSocketLocation("MuzzleFlash"));
	if(auto hitCharacter = Cast<AAICharacter>(HitResult.HitObjectHandle.FetchActor()))
	{
		if(!hitCharacter->isAlly())
		{
			hitCharacter->loseHealth(GunDamage);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("hit character"));	
		}
		
	}
	
}

void AAICharacter::BoostSpeed()
{
	// Set Max walking speed to 800
	GetCharacterMovement()->MaxWalkSpeed = 800.f;

	GetWorld()->GetTimerManager().SetTimer(BoostSpeedTimer, [&]()
		{
			// Set Max walking speed to 500
			GetCharacterMovement()->MaxWalkSpeed = 500.f;
			
			// Clear existing timer boost speed
			GetWorldTimerManager().ClearTimer(BoostSpeedTimer);

		}, 4, false);
}

void AAICharacter::RemoveSpeedBoost()
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void AAICharacter::FireParticle(FVector Start, FHitResult &Impact,FVector particleStart)
{
	if (!ParticleStart || !ParticleImpact) return;
	
	//FTransform ParticleT;

	//P//articleT.SetLocation(Start);

	//ParticleT.SetScale3D(FVector(0.25, 0.25, 0.25));

	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ParticleStart, particleStart,GetActorRotation());

	// Spawn particle at impact point
	//ParticleT.SetLocation(Impact);

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Impact.ImpactPoint.ToString());
	const FVector impact = FVector(Impact.ImpactPoint.X,Impact.ImpactPoint.Y,Impact.ImpactPoint.Z);
	const TConstArrayView<FVector> points = {Start,impact};
	TArray<FVector> test = {Start,impact};
	DrawCentripetalCatmullRomSpline(GetWorld(),points,FColor::Blue,0.5,8,false,2,0,2);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ParticleImpact, Impact.ImpactPoint,GetActorRotation());
}

FVector AAICharacter::FindNextWanderPoint()
{
	
	FVector Center = Owner->GetActorLocation() + Owner->GetActorForwardVector() * wanderDistance;

	// calculate the displacement point
	FVector Displacement = FVector(0, 1, 0) * wanderRadius;
	// randomly change the vector direction by making it change its current angle
	//Displacement = Displacement.RotateAngleAxis(WanderTheta, FVector(0, 0, 1));
	// change wander theta
	//WanderTheta += FMath::RandRange(-WanderJitter, WanderJitter);
	// calculate the wander force
	FVector Target = Center + Displacement;
	Target.Z = 0;
	// return zero vector
	//return Seek(Target);
	return Displacement;
}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

