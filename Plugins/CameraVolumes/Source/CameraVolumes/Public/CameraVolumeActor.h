// redbox, 2024

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

UCLASS(Config = CameraVolumes, AutoExpandCategories = ("Camera", "Camera|Location", "Camera|Rotation", "Camera|FOV/OrthoWidth", "Camera|Utils", "Volume", "VolumeSides", "VolumeSides|SmoothTransition", "VolumeSides|Right", "VolumeSides|Left", "VolumeSides|Top", "VolumeSides|Bottom", "VolumeSides|Front", "VolumeSides|Back"))
class CAMERAVOLUMES_API ACameraVolumeActor : public AActor
{
	GENERATED_BODY()

public:
	ACameraVolumeActor();


	// Components
protected:
	UPROPERTY()
	class USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, Category = Volume)
	class UBoxComponent* BoxComponent;


#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	class UBillboardComponent* BillboardComponent;

	UPROPERTY(Config)
	FString BillboardIconPath;

	UPROPERTY()
	class UCameraVolumesCameraComponent* CameraPreview;
#endif


	// Volume
public:
	// Camera volume priority (to handle overlapped volumes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume, Meta = (ClampMin = "-100", ClampMax = "100", UIMin = "-100", UIMax = "100"))
	int32 Priority;

protected:
	UPROPERTY(Config)
	float OpenEdgeOffset;

	UPROPERTY(Config)
	FVector VolumeExtentDefault;

public:
	// Volume extent
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (MakeEditWidget = true))
	FVector VolumeExtent;

	// Set new VolumeExtent
	UFUNCTION(BlueprintCallable, Category = Volume)
	void SetVolumeExtent(FVector NewVolumeExtent);


	// Depth extent
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bUseZeroDepthExtentEditCond;
#endif

public:
	// (For 2D games) Should use zero volume extent (by depth) for camera blocking?
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (EditCondition = "bUseZeroDepthExtentEditCond"))
	bool bUseZeroDepthExtent;

	// Set new bUseZeroDepthExtent
	UFUNCTION(BlueprintCallable, Category = Volume)
	void SetUseZeroDepthExtent(bool bNewUseZeroDepthExtent);


	// 6 DOF volume
public:
	// Should process all 6 volume sides?
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume)
	bool bUse6DOFVolume;

	// Set new bUse6DOFVolume
	UFUNCTION(BlueprintCallable, Category = Volume)
	void SetUse6DOFVolume(bool bNewUse6DOFVolume);


	// Camera blocking
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bPerformCameraBlockingEditCond;
#endif

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
	void SetDisableMainBoxCollision(bool bNewDisableMainBoxCollision);


#if WITH_EDITORONLY_DATA
public:
	// Camera projection mode (for camera frustrum preview only!)
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = Camera)
	TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionMode;
#endif


	// Camera mobility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
	ECameraMobility CameraMobility;

	// Set new CameraMobility
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetCameraMobility(ECameraMobility NewCameraMobility);

protected:
	UPROPERTY()
	bool bIsCameraStatic;

public:
	// Returns is volume uses static camera mobility
	UFUNCTION(BlueprintPure, Category = Camera)
	bool GetIsCameraStatic() const { return bIsCameraStatic; }


	// Camera rotation axis
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bUseCameraRotationAxisEditCond;
#endif

public:
	// Should rotate camera around volume's central axis?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bUseCameraRotationAxisEditCond"))
	bool bUseCameraRotationAxis;


	// Camera location
public:
	// Should override camera location?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Location")
	bool bOverrideCameraLocation;

	// Set new bOverrideCameraLocation
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetOverrideCameraLocation(bool bNewOverrideCameraLocation);

	// New camera location
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Location", Meta = (EditCondition = "bOverrideCameraLocation", MakeEditWidget = true))
	FVector CameraLocation;

	// Set new CameraLocation
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetCameraLocation(FVector NewCameraLocation);

	// Should camera location be relative to volume or to player character?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Location")
	bool bCameraLocationRelativeToVolume;


	// Camera rotation
public:
	// Should override camera focal point?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Rotation")
	bool bOverrideCameraRotation;

	// Set new bOverrideCameraRotation
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetOverrideCameraRotation(bool bNewOverrideCameraRotation);

	// New camera focal point
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Rotation", Meta = (EditCondition = "bOverrideCameraRotation", MakeEditWidget = true))
	FVector CameraFocalPoint;

	// Set new CameraFocalPoint
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetCameraFocalPoint(FVector NewCameraFocalPoint);

	// New camera roll
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Rotation", Meta = (EditCondition = "bOverrideCameraRotation"))
	float CameraRoll;

	// Set new CameraRoll
	UFUNCTION(BlueprintCallable, Category = Camera)
	void SetCameraRoll(float NewCameraRoll);

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bFocalPointIsPlayerEditCond;
#endif

public:
	// Should camera look at player character?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Rotation", Meta = (EditCondition = "bFocalPointIsPlayerEditCond"))
	bool bFocalPointIsPlayer;

protected:
	UPROPERTY()
	FQuat CameraRotation;

public:
	const FQuat& GetCameraRotation() const { return CameraRotation; }

	// Camera FOV/OrthoWidth
public:
	// Should override camera FOV? For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV/OrthoWidth", Meta = (EditCondition = "CameraProjectionMode == ECameraProjectionMode::Perspective"))
	bool bOverrideCameraFieldOfView;

	// New camera FOV. For perspective cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV/OrthoWidth", Meta = (EditCondition = "bOverrideCameraFieldOfView", UIMin = "5", UIMax = "170", ClampMin = "0.001", ClampMax = "360", Units = deg))
	float CameraFieldOfView;

	// Should override camera OrthoWidth? For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV/OrthoWidth", Meta = (EditCondition = "CameraProjectionMode == ECameraProjectionMode::Orthographic"))
	bool bOverrideCameraOrthoWidth;

	// New camera OrthoWidth. For orthographic cameras.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV/OrthoWidth", Meta = (EditCondition = "bOverrideCameraOrthoWidth"))
	float CameraOrthoWidth;


	// Camera lag
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bDisableCameraLocationLagEditCond;

	UPROPERTY()
	bool bDisableCameraRotationLagEditCond;
#endif

public:
	// Should disable camera location lag in this particular volume?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Utils", Meta = (EditCondition = "bDisableCameraLocationLagEditCond"))
	bool bDisableCameraLocationLag;

	// Should disable camera rotation lag in this particular volume?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Utils", Meta = (EditCondition = "bDisableCameraRotationLagEditCond"))
	bool bDisableCameraRotationLag;

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


	// Camera collision
#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY()
	bool bDoCollisionTestEditCond;
#endif

public:
	// Should do a camera collision test in this particular volume?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Utils", Meta = (EditCondition = "bDoCollisionTestEditCond"))
	bool bDoCollisionTest;


	// Sides info
public:
	// Right side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Right", Meta = (ShowOnlyInnerProperties))
	FSideInfo RightSide;

	// Left side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Left", Meta = (ShowOnlyInnerProperties))
	FSideInfo LeftSide;

	// Top side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Top", Meta = (ShowOnlyInnerProperties))
	FSideInfo TopSide;

	// Bottom side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Bottom", Meta = (ShowOnlyInnerProperties))
	FSideInfo BottomSide;

	// Front side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Front", Meta = (ShowOnlyInnerProperties))
	FSideInfo FrontSide;

	// Back side info
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VolumeSides|Back", Meta = (ShowOnlyInnerProperties))
	FSideInfo BackSide;

	// Set new side info
	UFUNCTION(BlueprintCallable, Category = VolumeSides)
	void SetSide(ESide Side, FSideInfo NewSideInfo);

	// In-editor all sides setters
#if WITH_EDITOR
public:
	// Set all open
	UFUNCTION(CallInEditor, Category = VolumeSides)
	void SetAllOpen();

	// Set all closed
	UFUNCTION(CallInEditor, Category = VolumeSides)
	void SetAllClosed();

	// Set all normal
	UFUNCTION(CallInEditor, Category = VolumeSides)
	void SetAllNormal();

	// Set all smooth
	UFUNCTION(CallInEditor, Category = VolumeSides)
	void SetAllSmooth();

	// Set all cut
	UFUNCTION(CallInEditor, Category = VolumeSides)
	void SetAllCut();
#endif


	// Text indicators
#if WITH_EDITOR
protected:
	void CreateSidesIndicators();
	void UpdateSidesIndicators();
#endif

#if WITH_EDITORONLY_DATA
public:
	// Text size of sides indicators
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = VolumeSides)
	float TextSize;

protected:
	UPROPERTY()
	TArray<class UTextRenderComponent*> Text_Indicators;

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
	float GetCamVolAspectRatio() const { return CamVolAspectRatio; }
	const FVector& GetCamVolMinCorrected() const { return CamVolMinCorrected; }
	const FVector& GetCamVolMaxCorrected() const { return CamVolMaxCorrected; }
	const FVector& GetCamVolExtentCorrected() const { return CamVolExtentCorrected; }

	// Use this to update volume after made changes in editor, if they are not applied automatically
	UFUNCTION(CallInEditor, Category = Volume)
	void UpdateVolume();

	// Calculate volume extents
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	void CalculateVolumeExtents();

	// Get side info for the volume side nearest to player location
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
	FSideInfo GetNearestVolumeSideInfo(const FVector& PlayerPawnLocation) const;

#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyMirror(const FVector& MirrorScale, const FVector& PivotLocation) override;
#endif
};
