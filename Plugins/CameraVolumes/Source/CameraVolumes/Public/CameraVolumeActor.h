//Dmitriy Barannik aka redbox, 2019

/**
* Camera volume actor stores desired camera parameters and volume information.
* Calculates camera preview, side indicators and volume extents by itself when updated in Editor.
* This actor and is static in context of this algorithm, but it's parameters can be changed in runtime.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "Components/BillboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "CameraVolumesTypes.h"
#include "CameraVolumeActor.generated.h"

UCLASS(Config = CameraVolumes, AutoExpandCategories = (Volume, Camera))
class CAMERAVOLUMES_API ACameraVolumeActor : public AActor
{
	GENERATED_BODY()

public:
	ACameraVolumeActor();

protected:
	//Components
	UPROPERTY()
		class USceneComponent* DefaultSceneRoot;

	UPROPERTY()
		class UBoxComponent* BoxComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
		class UBillboardComponent* BillboardComponent;

	UPROPERTY()
		class UCameraComponent* CameraComponent;
#endif
	//--------------------------------------------------

public:
	//Parameters
	/** Priority of camera volume in case of few overlapped volumes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume, Meta = (ClampMin = "-100", ClampMax = "100", UIMin = "-100", UIMax = "100"))
		int32 Priority;

	/** Volume extent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (MakeEditWidget = true))
		FVector VolumeExtent;

	/** Set new VolumeExtent */
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetVolumeExtent(FVector NewVolumeExtent);

protected:
	UPROPERTY()
		bool bUseZeroDepthExtentEditCond;

public:
	/** (For 2D games) Should use zero volume extent by depth for camera blocking? */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume, Meta = (EditCondition = "bUseZeroDepthExtentEditCond"))
		bool bUseZeroDepthExtent;

	/** Set new bUseZeroDepthExtent */
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetUseZeroDepthExtent(bool bNewUseZeroDepthExtent);

	/** Should process all 6 volume sides? */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = Volume)
		bool bUse6DOFVolume;

	/** Set new bUse6DOFVolume */
	UFUNCTION(BlueprintCallable, Category = Volume)
		virtual void SetUse6DOFVolume(bool bNewUse6DOFVolume);

	/** Should perform camera blocking calculations in this volume? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Volume)
		bool bPerformCameraBlocking;

#if WITH_EDITORONLY_DATA
	/** Camera projection mode (For camera frustrum preview only!) */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = Camera)
		TEnumAsByte<ECameraProjectionMode::Type> CameraProjectionMode;
#endif

	/** Camera mobility */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		ECameraMobility CameraMobility;

	/** Set new CameraMobility */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraMobility(ECameraMobility NewCameraMobility);

protected:
	UPROPERTY()
		bool bIsCameraStatic;

public:
	/** Returns is volume uses static camera settings */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		bool GetIsCameraStatic() const { return bIsCameraStatic; }

	/** Should override camera location? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		bool bOverrideCameraLocation;

	/** Set new bOverrideCameraLocation */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetOverrideCameraLocation(bool bNewOverrideCameraLocation);

	/** New camera location */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraLocation", MakeEditWidget = true))
		FVector CameraLocation;

	/** Set new CameraLocation */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraLocation(FVector NewCameraLocation);

	/** Should camera location be relative to volume? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bCameraLocationRelativeToVolume;

	/** Should override camera focal point? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		bool bOverrideCameraRotation;

	/** Set new bOverrideCameraRotation */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetOverrideCameraRotation(bool bNewOverrideCameraRotation);

	/** New camera focal point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraRotation", MakeEditWidget = true))
		FVector CameraFocalPoint;

	/** Set new CameraFocalPoint */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraFocalPoint(FVector NewCameraFocalPoint);

	/** New camera roll */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, Meta = (EditCondition = "bOverrideCameraRotation"))
		float CameraRoll;

	/** Set new CameraRoll */
	UFUNCTION(BlueprintCallable, Category = Camera)
		virtual void SetCameraRoll(float NewCameraRoll);

	/** Should camera look at player character? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bIsCameraStatic"))
		bool bFocalPointIsPlayer;

	UPROPERTY()
		FQuat CameraRotation;

	/** Should override camera FOV? For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bOverrideCameraFieldOfView;

	/** New camera FOV. For perspective cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bOverrideCameraFieldOfView", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
		float CameraFieldOfView;

	/** Should override camera OrthoWidth? For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
		bool bOverrideCameraOrthoWidth;

	/** New camera OrthoWidth. For orthographic cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, Meta = (EditCondition = "bOverrideCameraOrthoWidth"))
		float CameraOrthoWidth;

	/** Speed of smooth camera transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SidesInfo, Meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0"))
		float CameraSmoothTransitionSpeed;

	/** Right side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo RightSide;

	/** Set new RightSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetRightSide(FSideInfo NewRightSide);

	/** Left side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo LeftSide;

	/** Set new LeftSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetLeftSide(FSideInfo NewLeftSide);

	/** Top side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo TopSide;

	/** Set new TopSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetTopSide(FSideInfo NewTopSide);

	/** Bottom side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo BottomSide;

	/** Set new BottomSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetBottomSide(FSideInfo NewBottomSide);

	/** Front side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo FrontSide;

	/** Set new FrontSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetFrontSide(FSideInfo NewFrontSide);

	/** Back side info */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = SidesInfo, Meta = (ShowOnlyInnerProperties))
		FSideInfo BackSide;

	/** Set new BackSide info */
	UFUNCTION(BlueprintCallable, Category = SidesInfo)
		virtual void SetBackSide(FSideInfo NewBackSide);
	//--------------------------------------------------

	UPROPERTY()
		float CamVolAspectRatio;

	UPROPERTY()
		FVector CamVolWorldMin;

	UPROPERTY()
		FVector CamVolWorldMax;

	UPROPERTY()
		FVector CamVolWorldMinCorrected;

	UPROPERTY()
		FVector CamVolWorldMaxCorrected;

	UPROPERTY()
		FVector CamVolExtentCorrected;

	/** Use this to update volume after made changes in editor, if they are not applied automatically */
	UFUNCTION(CallInEditor, Category = Volume)
		virtual void UpdateVolume();

	/** Calculate volume extents */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual void CalculateVolumeExtents();

	/** Get side info */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual FSideInfo GetSideInfo(ESide Side);

	/** Get volume side nearest to player location */
	UFUNCTION(BlueprintCallable, Category = CameraVolumes)
		virtual ESide GetNearestVolumeSide(FVector& PlayerPawnLocation);

protected:
	const float OpenEdgeOffset = 10000.f;
	const FVector VolumeExtentDefault = FVector(500.f, 500.f, 500.f);

#if WITH_EDITOR
	UFUNCTION()
		virtual void CreateSidesIndicators();

	UFUNCTION()
		virtual void UpdateSidesIndicators();
#endif

#if WITH_EDITORONLY_DATA
	//Sides indicators
	UPROPERTY()
		TArray<UTextRenderComponent*> Text_Indicators;

public:
	/** Location of sides indicators. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = SidesInfo)
		ESideIndicatorsLocation IndicatorsLocation;

protected:
	UPROPERTY()
		bool bIndicatorsLocationOnSide;

public:
	/** Text size of sides indicators. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = SidesInfo)
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
	//--------------------------------------------------
#endif

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
	virtual void EditorApplyMirror(const FVector& MirrorScale, const FVector& PivotLocation) override;
#endif
};