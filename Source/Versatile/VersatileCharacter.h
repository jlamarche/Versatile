// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "VersatileCharacter.generated.h"


// MARK: - Camera Mode Enumeration

UENUM(BlueprintType)
namespace ECharacterCameraMode
{
	enum    Type
	{
		ThirdPersonDefault			UMETA(DisplayName="Third Person (Default UE4)"),
		FirstPerson					UMETA(DisplayName="First Person"),
		ThirdPersonSmoothFollow     UMETA(DisplayName="Third Person Smooth Follow"),
		ThirdPersonOverShoulder		UMETA(DisplayName="Third Person Over Shoulder"),
		
		Max							UMETA(Hidden),
	};
}

static inline bool IsFirstPerson(const ECharacterCameraMode::Type CameraMode)
{
	return (CameraMode == ECharacterCameraMode::FirstPerson);
}
static inline bool IsThirdPerson(const ECharacterCameraMode::Type CameraMode)
{
	return  !IsFirstPerson(CameraMode);
}
static inline FString GetNameForCameraMode(const ECharacterCameraMode::Type CameraMode)
{
	switch(CameraMode)
	{
		case ECharacterCameraMode::ThirdPersonDefault:
			return TEXT("Third Person (Default UE4)");
			break;
		case ECharacterCameraMode::FirstPerson:
			return TEXT("First Person");
			break;
		case ECharacterCameraMode::ThirdPersonSmoothFollow:
			return TEXT("Third Person Smooth Follow");
			break;
		case ECharacterCameraMode::ThirdPersonOverShoulder:
			return TEXT("Third Person Over Shoulder");
			break;
		default:
			return TEXT("Unknown Camera Mode");
	}
	return TEXT("");
}

// MARK: - AVersatileCharacter

UCLASS(config=Game)
class AVersatileCharacter : public ACharacter
{
	GENERATED_BODY()
	
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* ArmsMesh;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCamera;
	
public:
	
	/** The current camera mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	TEnumAsByte<ECharacterCameraMode::Type> CameraModeEnum;
	
	/** Controls the follow camera turn angle. Only affects Third Person Follow mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
	float CameraFollowTurnAngleExponent;
	
	/** Controls the follow camera turn speed. Only affects Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
	float CameraFollowTurnRate;
	
	/** Controls the speed that the camera resets in Third Person Follow mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCamera)
	float CameraResetSpeed;
	
	/** Minimum distance for follow cameras */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
	float CameraZoomMinimumDistance;
	
	/** Minimum distance for follow cameras */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
	float CameraZoomMaximumDistance;
	
	/** Zoom increment for follow cameras */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=CameraZoom)
	float CameraZoomIncrement;
	
	/** Whether the smooth follow camera should be reset to behind character after being idle for a time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
	bool AutoResetSmoothFollowCameraWhenIdle;
	
	/** The delay to use if using Auto Reset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
	float AutoResetDelaySeconds;
	
	/** The speed to use for Auto Resets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SmoothFollowCameraReset)
	float AutoResetSpeed;
	
protected:
	/** Keeps track of whether the camera is currently being reset */
	UPROPERTY(Transient)
	bool bIsResetting;
	
	/** Controls the speed that the camera resets in Third Person Follow mode */
	UPROPERTY(Transient)
	float CameraZoomCurrent;
	
	/** Keeps track of when the last movement was */
	UPROPERTY(Transient)
	float LastMovementTime;
	
	/** Keeps track of whether a reset is automatic or manual */
	UPROPERTY(Transient)
	bool IsAutoReset;
	
	
public:
	AVersatileCharacter();
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	/**
	 * Handles the Turn Axis, handles both third and first person movement.
	 * @param turnInput	The "Turn" axis value
	 */
	void HandleYawInput(float turnInput);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
	/** Handles setting of properties based on camera mode value */
	void UpdateForCameraMode();
	
	/** Cycles to the next camera mode */
	void CycleCamera();
	
	/**
	 * Sets the camera mode to a specific value and updates mesh visibility for the new camera mode.
	 * @param newCameraMode	The new camera mode value
	 */
	void SetCameraMode(ECharacterCameraMode::Type newCameraMode);
	
	/**
	 * Whether the current camera mode is a first person mode.
	 */
	bool IsInFirstPersonMode();
	
	/**
	 * Whether the current camera mode is a third person mode.
	 */
	bool IsInThirdPersonMode();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	virtual void PostInitializeComponents();
	virtual void Tick(float Delay);
	
private:
	void _ResettingTick(float DeltaSeconds)
	void _SmoothFollowTick(float DeltaSeconds);
};

