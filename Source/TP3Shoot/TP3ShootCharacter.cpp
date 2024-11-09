// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP3ShootCharacter.h"

#include "AICharacter.h"
#include "TimerManager.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <Perception/AISense_Sight.h>
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"


//////////////////////////////////////////////////////////////////////////
// ATP3ShootCharacter

ATP3ShootCharacter::ATP3ShootCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
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

	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimulis"));

	StimuliSource->RegisterForSense(TSubclassOf<UAISense_Sight>());
	StimuliSource->RegisterWithPerceptionSystem();
}


int ATP3ShootCharacter::getStartingHealth()
{
	return StartingHealth;
}

void ATP3ShootCharacter::loseHealth(int amount)
{
	Health -= amount;
	if( Health <= 0)
	{
		Destroy();
	}
}

int ATP3ShootCharacter::getHealth()
{
	return Health;
}

void ATP3ShootCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = StartingHealth;
	spawnPoint = GetActorLocation();
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATP3ShootCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ATP3ShootCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ATP3ShootCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ATP3ShootCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ATP3ShootCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATP3ShootCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATP3ShootCharacter::TouchStopped);

	// Aiming 
	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ATP3ShootCharacter::Aim);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ATP3ShootCharacter::StopAiming);

	// Fire
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATP3ShootCharacter::Fire);

	// Boost Speed
	PlayerInputComponent->BindAction("BoostSpeed", IE_Pressed, this, &ATP3ShootCharacter::BoostSpeed);
	PlayerInputComponent->BindAction("BoostSpeed", IE_Released, this, &ATP3ShootCharacter::RemoveSpeedBoost);
}

void ATP3ShootCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ATP3ShootCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ATP3ShootCharacter::Aim()
{
	IsAiming = true;
}

void ATP3ShootCharacter::StopAiming()
{
	IsAiming = false;
}

void ATP3ShootCharacter::Fire()
{
	FVector Start, LineTraceEnd, ForwardVector;
	FHitResult HitResult;
	
	if (IsAiming)
	{

		Start = FollowCamera->GetComponentLocation();

		ForwardVector = FollowCamera->GetForwardVector();

		LineTraceEnd = Start + (ForwardVector * 10000);
	}
	else {

		// Get muzzle location
		Start = SK_Gun->GetSocketLocation("MuzzleFlash");

		// Get Rotation Forward Vector
		ForwardVector = FollowCamera->GetForwardVector();

		// Get End Point
		LineTraceEnd = Start + (ForwardVector * 10000);
	}

	
	bool bSuccess = Controller->GetWorld()->LineTraceSingleByChannel(HitResult,Start,LineTraceEnd,ECollisionChannel::ECC_WorldDynamic);
	FireParticle(Start,HitResult,SK_Gun->GetSocketLocation("MuzzleFlash"));
	if(auto hitCharacter = Cast<AAICharacter>(HitResult.HitObjectHandle.FetchActor()))
	{
		if(!hitCharacter->isAlly())
		{
			hitCharacter->loseHealth(GunDamage);
			hitCharacter->setShooter(this);
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("hit character"));	
		}
		
	}
	
}



void ATP3ShootCharacter::BoostSpeed()
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

void ATP3ShootCharacter::RemoveSpeedBoost()
{
	// Set Max walking speed to 500
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
}


void ATP3ShootCharacter::FireParticle(FVector Start, FHitResult &Impact, FVector particleStart)
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

void ATP3ShootCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATP3ShootCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATP3ShootCharacter::MoveForward(float Value)
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

void ATP3ShootCharacter::MoveRight(float Value)
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
