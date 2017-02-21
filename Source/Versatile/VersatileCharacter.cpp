// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.


#include "Versatile.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "VersatileCharacter.h"
#include "Engine.h"

//////////////////////////////////////////////////////////////////////////
// AVersatileCharacter

AVersatileCharacter::AVersatileCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	SetRootComponent(GetCapsuleComponent());

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->GravityScale = 1.5f;
	MoveComp->JumpZVelocity = 620;
	MoveComp->MaxWalkSpeedCrouched = 200;
	MoveComp->bOrientRotationToMovement = true; // Character moves in the direction of input...
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	MoveComp->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// First person camera and arms
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	ArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	ArmsMesh->SetOnlyOwnerSee(true);
	ArmsMesh->SetupAttachment(FirstPersonCamera);
	ArmsMesh->bCastDynamicShadow = false;
	ArmsMesh->CastShadow = false;
	ArmsMesh->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	ArmsMesh->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	OverShoulderCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("ShoulderCameraBoom"));
	OverShoulderCameraBoom->SetupAttachment(GetCapsuleComponent());
	OverShoulderCameraBoom->bUsePawnControlRotation = false; // Fixed Camera
	OverShoulderCameraBoom->TargetArmLength = 75.0f;
	
	// Create a follow camera
	OverShoulderCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ShoulderCamera"));
	OverShoulderCamera->SetupAttachment(OverShoulderCameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	OverShoulderCamera->bUsePawnControlRotation = false;
	
	USkeletalMeshComponent *Mesh = GetMesh();
	Mesh->bCastDynamicShadow = true;
	Mesh->CastShadow = true;
	
	CameraFollowTurnAngleExponent = .25f;
	CameraFollowTurnRate = .3f;
	CameraResetSpeed = 1.f;
	
	CameraModeEnum = ECharacterCameraMode::ThirdPersonDefault;
	
	bIsResetting = false;
	
	CameraZoomMaximumDistance = 600.f;
	CameraZoomMinimumDistance = 100.f;
	CameraZoomCurrent = 300.f;
	CameraZoomIncrement = 20.f;
	CameraBoom->TargetArmLength = CameraZoomCurrent;
	
	AutoResetSmoothFollowCameraWhenIdle = true;
	AutoResetDelaySeconds = 2.5f;
	
	IsAutoReset = false;
	AutoResetSpeed = .15f;
	
}

//////////////////////////////////////////////////////////////////////////
// Input

void AVersatileCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVersatileCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVersatileCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AVersatileCharacter::HandleYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVersatileCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVersatileCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AVersatileCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AVersatileCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AVersatileCharacter::OnResetVR);
	
	// Camera
	InputComponent->BindAction("ToggleCameraMode", IE_Pressed, this, &AVersatileCharacter::CycleCamera);
}

void AVersatileCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UpdateForCameraMode();
}

void AVersatileCharacter::CycleCamera()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Cycle Camera Requested");
	int newCameraMode = (int)CameraModeEnum + 1;
	
	if (newCameraMode >= ECharacterCameraMode::Max) newCameraMode = ECharacterCameraMode::ThirdPersonDefault;
	SetCameraMode((ECharacterCameraMode::Type)newCameraMode);
}


void AVersatileCharacter::SetCameraMode(ECharacterCameraMode::Type newCameraMode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, "Setting Camera Mode to " + GetNameForCameraMode(newCameraMode));
	CameraModeEnum = newCameraMode;
	UpdateForCameraMode();
	
}

void AVersatileCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AVersatileCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
	LastMovementTime = FApp::GetCurrentTime();
}

void AVersatileCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
	LastMovementTime = FApp::GetCurrentTime();
}

void AVersatileCharacter::TurnAtRate(float Rate)
{
	if (Rate == 0.f || Controller == NULL) return;
	
	// calculate delta for this frame from the rate information
	if (!bIsResetting)
	{
		
		AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
		LastMovementTime = FApp::GetCurrentTime();
	}
}

void AVersatileCharacter::LookUpAtRate(float Rate)
{
	if (Rate == 0.f || Controller == NULL) return;
	
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	LastMovementTime = FApp::GetCurrentTime();
}

void AVersatileCharacter::MoveForward(float Value)
{
	if (Value == 0.f || Controller == NULL) return;
	
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
	
	LastMovementTime = FApp::GetCurrentTime();
}

void AVersatileCharacter::MoveRight(float Value)
{
	if (Value == 0.f || Controller == NULL) return;
	
	// find out which way is right
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	// get right vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// add movement in that direction
	AddMovementInput(Direction, Value);
	
	LastMovementTime = FApp::GetCurrentTime();
}

void AVersatileCharacter::HandleYawInput(float turnInput)
{
	if (!bIsResetting)
	{
		if (turnInput != 0.f)
		{
			AddControllerYawInput(turnInput);
			LastMovementTime = FApp::GetCurrentTime();
		}
	}
}

// MARK: - Camera Mode
void AVersatileCharacter::UpdateForCameraMode()
{
	// Changes visibility of first and third person meshes
	switch (CameraModeEnum)
	{
		case ECharacterCameraMode::ThirdPersonDefault:
			bIsResetting = false;
			// no break is intentional
		case ECharacterCameraMode::ThirdPersonSmoothFollow:
			GetMesh()->bOwnerNoSee = false;
			GetMesh()->MarkRenderStateDirty();
			ArmsMesh->bVisible = false;
			ArmsMesh->MarkRenderStateDirty();
			FollowCamera->SetActive(true);
			FirstPersonCamera->SetActive(false);
			GetCharacterMovement()->bOrientRotationToMovement = true;
			OverShoulderCamera->SetActive(false);
			break;
		case ECharacterCameraMode::FirstPerson:
			GetMesh()->bOwnerNoSee = true;
			GetMesh()->MarkRenderStateDirty();
			bIsResetting = false;
			ArmsMesh->bVisible = true;
			ArmsMesh->MarkRenderStateDirty();
			FollowCamera->SetActive(false);
			FirstPersonCamera->SetActive(true);
			OverShoulderCamera->SetActive(false);
			break;
			GetCharacterMovement()->bOrientRotationToMovement = false;
		case ECharacterCameraMode::ThirdPersonOverShoulder:
			bIsResetting = false;
			GetMesh()->bOwnerNoSee = false;
			GetMesh()->MarkRenderStateDirty();
			ArmsMesh->bVisible = false;
			ArmsMesh->MarkRenderStateDirty();
			FollowCamera->SetActive(false);
			FirstPersonCamera->SetActive(false);
			OverShoulderCamera->SetActive(true);
			GetCharacterMovement()->bOrientRotationToMovement = false;
			break;
		default:
			break;
	}
}

// MARK: - Tick
void AVersatileCharacter::_ResettingTick(float DeltaSeconds)
{
	const FRotator Rotation = Controller->GetControlRotation();
	FRotator meshRotation = GetMesh()->GetForwardVector().Rotation();
	meshRotation.Yaw += 90.f;
	meshRotation.Pitch = Controller->GetControlRotation().Pitch;
	
	float delta = meshRotation.Yaw - Rotation.Yaw;
	
	// Prevent going "the long way around"
	if (fabsf(delta) >= 180.f)
	{
		if (delta <= 0.f)
			delta +=360.f;
		else
			delta -= 360.f;
	}
	
	if (fabsf(delta) <= 1.f)
	{
		bIsResetting = false;
		IsAutoReset = false;
	}
	else
	{
		float resetSpeed = (IsAutoReset) ? AutoResetSpeed : CameraResetSpeed;
		AddControllerYawInput(delta * DeltaSeconds * resetSpeed);
	}
}

void AVersatileCharacter::_SmoothFollowTick(float DeltaSeconds)
{
	const FRotator Rotation = Controller->GetControlRotation();
	
	float currentTime = FApp::GetCurrentTime();
	
	if (currentTime > LastMovementTime + AutoResetDelaySeconds)
	{
		IsAutoReset = true;
		bIsResetting = true;
	}
	if (bIsResetting)
	{
		_ResettingTick(DeltaSeconds);
		return;
	}
	
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	float forwardAxis = GetInputAxisValue("MoveForward");
	float rightAxis = GetInputAxisValue("MoveRight");
	
	float inputVectorLength = FMath::Clamp((fabsf(forwardAxis) + fabsf(rightAxis)), 0.f, 1.f);
	
	if (inputVectorLength == 0.f) return;
	
	const FVector forward = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::X);
	const FVector right = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::Y);
	
	const FVector forwardVector = forward * forwardAxis;
	
	const FVector rightVector = right * rightAxis;
	const FVector combinedVector = (forwardVector + rightVector).GetSafeNormal();
	
	// 1 = character is perpindicular to camera vector, 0 when parallel to camera vector
	float dotProduct = FMath::Pow(1 - fabsf(FVector::DotProduct(forwardVector, combinedVector)), CameraFollowTurnAngleExponent);
	FRotator delta = (combinedVector.Rotation() - Rotation);
	delta.Normalize();
	AddControllerYawInput(FMath::Clamp(inputVectorLength * DeltaSeconds * dotProduct * CameraFollowTurnRate, 0.f, 1.f) * delta.Yaw);
}
void AVersatileCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (CameraModeEnum == ECharacterCameraMode::ThirdPersonSmoothFollow)
	{
		_SmoothFollowTick(DeltaSeconds);
	}
	
}
