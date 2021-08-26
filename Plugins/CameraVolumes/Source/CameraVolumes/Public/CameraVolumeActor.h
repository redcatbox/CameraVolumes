// redbox, 2021

/**
 * Camera volume actor stores desired camera parameters and volume information.
 * Calculates camera preview, side indicators and volume extents by itself when updated in Editor.
 * This actor is static in context of this algorithm, but it's parameters can be changed during runtime.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraVolumesTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "CameraVolumeActor.generated.h"

UCLASS(Config = CameraVolumes, AutoExpandCategories = (Camera, Volume, VolumeSides, "VolumeSides|SmoothTransition", DeadZone))
class CAMERAVOLUMES_API ACameraVolumeActor : public AActor
{
	GENERATED_BODY()

public:
	ACameraVolumeActor();


//Components
protected:
	UPROPERTY()
		class USceneComponent* DefaultSceneRoot;

	UPROPERTY()
		class UBoxComponent* BoxComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		class UBillboardComponent* BillboardComponent;

	UPROPERTY(Config)
		FString BillboardIconPath;

	UPROPERTY()
		class UCameraVolumesCameraComponent* CameraPreview;
#endif


public:
	// Priority of camera volume in case of few overlapped volumes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume, Meta = (ClampMin = "-100", ClampMax = "100", UIMin = "-100", UIMax = "100"))
		int32 Priority;


	// Volume extent
protected:
	UPROPERTY(Config)
		float OpenEdgeOffset;

	UPROPERTY(Config)
		FVector VolumeExtentDefault;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (MakeEditWidget = true))
		FVector VolumeExtent;

	// Set new VolumeExtent
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetVolumeExtent(FVector NewVolumeExtent);


	// Depth extent
protected:
	UPROPERTY()
		bool bUseZeroDepthExtentEditCond;

public:
	// (For 2D games) Should use zero volume extent by depth for camera blocking?
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (EditCondition = "bUseZeroDepthExtentEditCond"))
		bool bUseZeroDepthExtent;

	// Set new bUseZeroDepthExtent
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetUseZeroDepthExtent(bool bNewUseZeroDepthExtent);


	// 6 DOF volume
public:
	// Should process all 6 volume sides?
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume)
		bool bUse6DOFVolume;

	// Set new bUse6DOFVolume
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetUse6DOFVolume(bool bNewUse6DOFVolume);


	// Camera blocking
protected:
	UPROPERTY()
		bool bPerformCameraBlockingEditCond;

public:
	// Should perform camera blocking calculations in this volume?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume, Meta = (EditCondition = "bPerformCameraBlockingEditCond"))
		bool bPerformCameraBlocking;


	// Main box collision
public:
	// Should disable collision for main box primitive?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volume)
		bool bDisableMainBoxCollision;

	// Disable main box collision
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetDisableMainBoxCollision(bool bNewDisableMainBoxCollision);


public:
#if WITH_EDITORONLY_DATA
	// Camera projection mode (For camera frustrum preview only!)
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = Camera)
		TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionMode;
#endif

	// Camera mobility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		ECameraMobility CameraMobility;

	// Set new CameraMobility
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraMobility(ECameraMobility NewCameraMobility);

protected:
	UPROPERTY()
		bool bIsCameraStatic;

public:
	// Returns is volume uses static camera mobility
	UFUNCTION(BlueprintCallable, Category = Camera)
		bool GetIsCameraStatic() const;


	// Camera location
public:
	// Should override camera location?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		bool bOverrideCameraLocation;

	// Set new bOverrideCameraLocation
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetOverrideCameraLocation(bool bNewOverrideCameraLocation);

	// New camera location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraLocation", MakeEditWidget = true))
		FVector CameraLocation;

	// Set new CameraLocation
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraLocation(FVector NewCameraLocation);

	// Should camera location be relative to volume?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bCameraLocationRelativeToVolume;


	// Camera rotation
public:
	// Should override camera focal point?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		bool bOverrideCameraRotation;

	// Set new bOverrideCameraRotation
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetOverrideCameraRotation(bool bNewOverrideCameraRotation);

	// New camera focal point
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraRotation", MakeEditWidget = true))
		FVector CameraFocalPoint;

	// Set new CameraFocalPoint
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraFocalPoint(FVector NewCameraFocalPoint);

	// New camera roll
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraRotation"))
		float CameraRoll;

	// Set new CameraRoll
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraRoll(float NewCameraRoll);

protected:
	UPROPERTY()
		bool bFocalPointIsPlayerEditCond;

public:
	// Should camera look at player character?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bFocalPointIsPlayerEditCond"))
		bool bFocalPointIsPlayer;

	UPROPERTY()
		FQuat CameraRotation;


	// Camera FOV/OrthoWidth
public:
	// Should override camera FOV? For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bOverrideCameraFieldOfView;

	// New camera FOV. For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bOverrideCameraFieldOfView", UIMin = "5", UIMax = "170", ClampMin = "0.001", ClampMax = "360", Units = deg))
		float CameraFieldOfView;

	// Should override camera OrthoWidth? For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bOverrideCameraOrthoWidth;

	// New camera OrthoWidth. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bOverrideCameraOrthoWidth"))
		float CameraOrthoWidth;


	// Camera rotation axis
protected:
	UPROPERTY()
		bool bUseCameraRotationAxisEditCond;

public:
	// Should rotate camera around volume's central axis?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bUseCameraRotationAxisEditCond"))
		bool bUseCameraRotationAxis;

	// Speed of smooth camera transition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VolumeSides|SmoothTransition", Meta = (ClampMin = "0.01", ClampMax = "10", UIMin = "0.01", UIMax = "10"))
		float CameraSmoothTransitionSpeed;

	// Smooth camera transition easing function
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VolumeSides|SmoothTransition")
		TEnumAsByte<EEasingFunc::Type> SmoothTransitionEasingFunc;

	// Easing function exponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VolumeSides|SmoothTransition")
		float EasingFuncBlendExp;

	// Easing function steps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VolumeSides|SmoothTransition", Meta = (ClampMin = "1", UIMin = "1"))
		int32 EasingFuncSteps;
	
	// Sides info
public:
	// Right side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo RightSide;

	// Set new RightSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetRightSide(FSideInfo NewRightSide);

	// Left side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo LeftSide;

	// Set new LeftSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetLeftSide(FSideInfo NewLeftSide);

	// Top side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo TopSide;

	// Set new TopSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetTopSide(FSideInfo NewTopSide);

	// Bottom side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo BottomSide;

	// Set new BottomSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetBottomSide(FSideInfo NewBottomSide);

	// Front side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo FrontSide;

	// Set new FrontSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetFrontSide(FSideInfo NewFrontSide);

	// Back side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumeSides, Meta = (ShowOnlyInnerProperties))
		FSideInfo BackSide;

	// Set new BackSide info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
		virtual void SetBackSide(FSideInfo NewBackSide);


	// In-editor all sides setters
#if WITH_EDITOR
public:
	// Set all open
	UFUNCTION(CallInEditor, Category = VolumeSides)
		virtual void SetAllOpen();

	// Set all closed
	UFUNCTION(CallInEditor, Category = VolumeSides)
		virtual void SetAllClosed();

	// Set all normal
	UFUNCTION(CallInEditor, Category = VolumeSides)
		virtual void SetAllNormal();

	// Set all smooth
	UFUNCTION(CallInEditor, Category = VolumeSides)
		virtual void SetAllSmooth();

	// Set all cut
	UFUNCTION(CallInEditor, Category = VolumeSides)
		virtual void SetAllCut();
#endif


	// Text indicators
protected:
#if WITH_EDITOR
	UFUNCTION()
		virtual void CreateSidesIndicators();

	UFUNCTION()
		virtual void UpdateSidesIndicators();
#endif

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		TArray<class UTextRenderComponent*> Text_Indicators;

public:
	// Text size of sides indicators
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = VolumeSides)
		float TextSize;

protected:
	const FText Text_Front = FText::FromString("FRONT");
	const FText Text_Back = FText::FromString("BACK");
	const FText Text_Right = FText::FromString("RIGHT");
	const FText Text_Left = FText::FromString("LEFT");
	const FText Text_Top = FText::FromString("TOP");
	const FText Text_Bottom = FText::FromString("BOTTOM");
	const FText Text_Open = FText::FromString("OPEN");
	const FText Text_Closed = FText::FromString("CLOSED");
	const FText Text_Normal = FText::FromString("NORMAL");
	const FText Text_Smooth = FText::FromString("SMOOTH");
	const FText Text_Cut = FText::FromString("CUT");
#endif


	// Dead zone
public:
	// Should override dead zone settings?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone)
		bool bOverrideDeadZoneSettings;

	// Dead zone extent (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone, Meta = (EditCondition = "bOverrideDeadZoneSettings"))
		FVector2D DeadZoneExtent;

	// Dead zone offset from the center of the screen (in screen percentage)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DeadZone, Meta = (EditCondition = "bOverrideDeadZoneSettings"))
		FVector2D DeadZoneOffset;


protected:
	UPROPERTY()
		float CamVolAspectRatio;

	UPROPERTY()
		FVector CamVolMinCorrected;

	UPROPERTY()
		FVector CamVolMaxCorrected;

	UPROPERTY()
		FVector CamVolExtentCorrected;

public:
	float GetCamVolAspectRatio() const;
	FVector GetCamVolMinCorrected() const;
	FVector GetCamVolMaxCorrected() const;
	FVector GetCamVolExtentCorrected() const;

	// Use this to update volume after made changes in editor, if they are not applied automatically
	UFUNCTION(CallInEditor, Category = Volume)
		virtual void UpdateVolume();

	// Calculate volume extents
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void CalculateVolumeExtents();

	// Get side info for the volume side nearest to player location
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual FSideInfo GetNearestVolumeSideInfo(FVector& PlayerPawnLocation);

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyMirror(const FVector& MirrorScale, const FVector& PivotLocation) override;
#endif
};
