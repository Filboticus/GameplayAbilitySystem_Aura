// Copyright TBD


#include "Player/AuraPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);                

	CursorTrace();
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(AuraContext);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}
	
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2d InputAxisVector = InputActionValue.Get<FVector2d>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	
	GetHitResultUnderCursor(ECC_Visibility, false,CursorHit);
	if(!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	/**
	 * Line Trace from cursor. There are several scenaries:
	 * A. LastActor is null && ThisActor is null
	 *		-Do Nothing
	 * B. LastActor is null && ThisActor is valid
	 *		-Highlight ThisActor
	 * C. LastActor is valid && ThisActor is null
	 *		-UnHighlight LastActor
	 * D. Both Actors are valid, but LastActor != ThisActor
	 *		-UnHighlight LastActor, and Highlight ThisActor
	 * E. Both Actors are valid, and are the same Actor
	 *		-Do Nothing
	 */

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// Case B - LastActor is null && ThisActor is valid
			ThisActor->HighlightActor();
		}
		else
		{
		// Case A - LastActor is null && ThisActor is null
		}
	}
	else // LastActor is Valid
		{
			if(ThisActor == nullptr)
			{
				// Case C - LastActor is valid && ThisActor is null
				LastActor->UnHighlightActor();
			}
			else // Both Actors are Valid
			{
				if(LastActor != ThisActor)
				{
					// Case D - Both Actors are valid, but LastActor != ThisActor
					LastActor->UnHighlightActor();
					ThisActor->HighlightActor();
				}
				else
				{
				// Case E - Both Actors are valid, and are the same Actor
				}
			}
		}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(2, 3.f, FColor::Blue, *InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	GEngine->AddOnScreenDebugMessage(3, 3.f, FColor::Green, *InputTag.ToString());
}
