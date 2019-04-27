//Dmitriy Barannik aka redbox, 2019

#include "CameraVolumeActor.h"
#include "CameraVolumesFunctionLibrary.h"

ACameraVolumeActor::ACameraVolumeActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Default root
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	DefaultSceneRoot->Mobility = EComponentMobility::Static;
	DefaultSceneRoot->bVisible = false;
	RootComponent = DefaultSceneRoot;

#if WITH_EDITORONLY_DATA
	// Billboard
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(RootComponent);
	BillboardComponent->bHiddenInGame = true;
	static ConstructorHelpers::FObjectFinder<UTexture2D> TextureObj(TEXT("/CameraVolumes/Icons/CameraVolume"));
	if (TextureObj.Object)
		BillboardComponent->Sprite = TextureObj.Object;

	// CameraComponent
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
#endif

	// BoxComponent
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Default values
	Priority = 0;
	VolumeExtent = VolumeExtentDefault;

	bUseZeroDepthExtentEditCond = true;
	bUseZeroDepthExtent = false;
	bUse6DOFVolume = false;
	bPerformCameraBlocking = true;

#if WITH_EDITORONLY_DATA
	CameraProjectionMode = ECameraProjectionMode::Perspective;
#endif

	CameraMobility = ECameraMobility::ECM_Movable;

	bOverrideCameraLocation = false;
	bCameraLocationRelativeToVolume = true;

	bOverrideCameraRotation = false;
	CameraFocalPoint = FVector::ZeroVector;
	CameraRoll = 0.f; //Side-scroller
	//CameraRoll = 90.f; //Top-down
	bFocalPointIsPlayer = false;

	bOverrideCameraFieldOfView = false;
	CameraFieldOfView = 90.f;

	bOverrideCameraOrthoWidth = false;
	CameraOrthoWidth = 512.f;

	CameraSmoothTransitionSpeed = 1.f;

#if WITH_EDITORONLY_DATA
	TextSize = 50.f;
#endif

	RightSide.Side = ESide::ES_Right;
	LeftSide.Side = ESide::ES_Left;
	TopSide.Side = ESide::ES_Top;
	BottomSide.Side = ESide::ES_Bottom;
	FrontSide.Side = ESide::ES_Front;
	BackSide.Side = ESide::ES_Back;

#if WITH_EDITOR
	CreateSidesIndicators();
#endif

	UpdateVolume();
}

void ACameraVolumeActor::UpdateVolume()
{
	//Reset actor scale
	SetActorScale3D(FVector::OneVector);

	//Extents
	CalculateVolumeExtents();

	//Components
	BoxComponent->SetBoxExtent(VolumeExtent);

#if WITH_EDITORONLY_DATA
	BillboardComponent->SetRelativeLocation(FVector::ZeroVector);
	BillboardComponent->SetRelativeScale3D(FVector(5.f, 1.f, 1.f));
#endif

	if (bUse6DOFVolume)
	{
		bUseZeroDepthExtentEditCond = false;
		bUseZeroDepthExtent = false;
	}
	else
		bUseZeroDepthExtentEditCond = true;

	switch (CameraMobility)
	{
	case ECameraMobility::ECM_Movable:
		bIsCameraStatic = false;
		bFocalPointIsPlayer = true;
		break;
	case ECameraMobility::ECM_Static:
		bIsCameraStatic = true;
		bOverrideCameraLocation = true;
		bPerformCameraBlocking = false;
		break;
	}

	if (!bOverrideCameraLocation)
		CameraLocation = FVector(0.f, 1000.f, 0.f); //Side-scroller
		//CameraLocation = FVector(0.f, 0.f, 1000.f); //Top-down

	if (!bOverrideCameraRotation)
	{
		CameraFocalPoint = FVector::ZeroVector;
		CameraRoll = 0.f;
		CameraRotation = FRotator(0.f, -90.f, 0.f).Quaternion();
	}
	else
		CameraRotation = UCameraVolumesFunctionLibrary::CalculateCameraRotation(CameraLocation, CameraFocalPoint, CameraRoll);

	if (!bOverrideCameraFieldOfView)
		CameraFieldOfView = 90.f;

	if (!bOverrideCameraOrthoWidth)
		CameraOrthoWidth = 512.f;

#if WITH_EDITORONLY_DATA
	CameraComponent->SetRelativeLocationAndRotation(CameraLocation, CameraRotation);
	CameraComponent->FieldOfView = CameraFieldOfView;
	CameraComponent->OrthoWidth = CameraOrthoWidth;
	CameraComponent->ProjectionMode = CameraProjectionMode;
	CameraComponent->RefreshVisualRepresentation();
#endif

#if WITH_EDITOR
	UpdateSidesIndicators();
	this->Modify();
#endif
}

void ACameraVolumeActor::CalculateVolumeExtents()
{
	VolumeExtent = VolumeExtent.GetAbs();
	CamVolMinCorrected = -VolumeExtent;
	CamVolMaxCorrected = VolumeExtent;

	if (RightSide.SideType == ESideType::EST_Open)
		CamVolMaxCorrected.X += OpenEdgeOffset;

	if (LeftSide.SideType == ESideType::EST_Open)
		CamVolMinCorrected.X -= OpenEdgeOffset;

	//Side-scroller
	if (TopSide.SideType == ESideType::EST_Open)
		CamVolMaxCorrected.Z += OpenEdgeOffset;

	if (BottomSide.SideType == ESideType::EST_Open)
		CamVolMinCorrected.Z -= OpenEdgeOffset;

	//Top-down
	//if (FrontSide.SideType == ESideType::EST_Open)
	//	CamVolMaxCorrected.Y += OpenEdgeOffset;

	//if (BackSide.SideType == ESideType::EST_Open)
	//	CamVolMinCorrected.Y -= OpenEdgeOffset;

	if (bUse6DOFVolume)
	{
		//Side-scroller
		if (FrontSide.SideType == ESideType::EST_Open)
			CamVolMaxCorrected.Y += OpenEdgeOffset;

		if (BackSide.SideType == ESideType::EST_Open)
			CamVolMinCorrected.Y -= OpenEdgeOffset;

		//Top-down
		//if (TopSide.SideType == ESideType::EST_Open)
		//	CamVolMaxCorrected.Z += OpenEdgeOffset;

		//if (BottomSide.SideType == ESideType::EST_Open)
		//	CamVolMinCorrected.Z -= OpenEdgeOffset;
	}

	if (bUseZeroDepthExtent)
	{
		//Side-scroller
		CamVolMinCorrected.Y = 0.f;
		CamVolMaxCorrected.Y = 0.f;
		//Top-down
		//CamVolMinCorrected.Z = 0.f;
		//CamVolMaxCorrected.Z = 0.f;
	}

	CamVolExtentCorrected = (CamVolMaxCorrected - CamVolMinCorrected) * 0.5f;
	CamVolAspectRatio = CamVolExtentCorrected.X / CamVolExtentCorrected.Z; //Side-scroller
	//CamVolAspectRatio = CamVolExtentCorrected.X / CamVolExtentCorrected.Y; //Top-down
}

FSideInfo ACameraVolumeActor::GetNearestVolumeSideInfo(FVector& PlayerPawnLocation)
{
	ESide NearestSide = ESide::ES_Front;
	TMap<ESide, float> Sides;
	FVector PlayerPawnLocationTransformed = GetActorTransform().InverseTransformPositionNoScale(PlayerPawnLocation);

	if (bUse6DOFVolume)
	{
		Sides.Add(ESide::ES_Front, FMath::Abs(PlayerPawnLocationTransformed.Y - VolumeExtent.Y));
		Sides.Add(ESide::ES_Back, FMath::Abs(PlayerPawnLocationTransformed.Y + VolumeExtent.Y));
		Sides.Add(ESide::ES_Right, FMath::Abs(PlayerPawnLocationTransformed.X - VolumeExtent.X));
		Sides.Add(ESide::ES_Left, FMath::Abs(PlayerPawnLocationTransformed.X + VolumeExtent.X));
		Sides.Add(ESide::ES_Top, FMath::Abs(PlayerPawnLocationTransformed.Z - VolumeExtent.Z));
		Sides.Add(ESide::ES_Bottom, FMath::Abs(PlayerPawnLocationTransformed.Z + VolumeExtent.Z));
	}
	else
	{
		//Side-scroller
		Sides.Add(ESide::ES_Right, FMath::Abs(PlayerPawnLocationTransformed.X - VolumeExtent.X));
		Sides.Add(ESide::ES_Left, FMath::Abs(PlayerPawnLocationTransformed.X + VolumeExtent.X));
		Sides.Add(ESide::ES_Top, FMath::Abs(PlayerPawnLocationTransformed.Z - VolumeExtent.Z));
		Sides.Add(ESide::ES_Bottom, FMath::Abs(PlayerPawnLocationTransformed.Z + VolumeExtent.Z));
		//Top-down
		//Sides.Add(ESide::ES_Front, FMath::Abs(PlayerPawnLocationTransformed.Y - VolumeExtent.Y));
		//Sides.Add(ESide::ES_Back, FMath::Abs(PlayerPawnLocationTransformed.Y + VolumeExtent.Y));
		//Sides.Add(ESide::ES_Right, FMath::Abs(PlayerPawnLocationTransformed.X - VolumeExtent.X));
		//Sides.Add(ESide::ES_Left, FMath::Abs(PlayerPawnLocationTransformed.X + VolumeExtent.X));
	}

	Sides.ValueSort([](float Min, float Max) { return Min < Max; });

	for (auto& Pair : Sides)
	{
		NearestSide = Pair.Key;
		break;
	}

	switch (NearestSide)
	{
	case ESide::ES_Front:
		return FrontSide;
		break;
	case ESide::ES_Back:
		return BackSide;
		break;
	case ESide::ES_Right:
		return RightSide;
		break;
	case ESide::ES_Left:
		return LeftSide;
		break;
	case ESide::ES_Top:
		return TopSide;
		break;
	case ESide::ES_Bottom:
		return BottomSide;
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unknown side type! Using SideInfo(Front, Open, Normal)"));
		return FSideInfo();
		break;
	}
}

#if WITH_EDITOR
void ACameraVolumeActor::CreateSidesIndicators()
{
	/**
	*	Sides indicators
	*	Priority [0]
	*	SideInfo [1-18]
	*/
	Text_Indicators.SetNum(19);
	FName ComponentName;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TextMaterialObj(TEXT("/CameraVolumes/Materials/UnlitText.UnlitText"));
	for (uint8 i = 0; i < 19; i++)
	{
		ComponentName = *(FString(TEXT("TextRenderComponent")) + FString::FromInt(i));
		Text_Indicators[i] = CreateDefaultSubobject<UTextRenderComponent>(ComponentName);
		Text_Indicators[i]->SetupAttachment(RootComponent);
		Text_Indicators[i]->bHiddenInGame = true;
		Text_Indicators[i]->bAlwaysRenderAsText = true;
		Text_Indicators[i]->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
		Text_Indicators[i]->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
		Text_Indicators[i]->SetTextRenderColor(FColor::White);
		if (TextMaterialObj.Object)
			Text_Indicators[i]->SetTextMaterial(TextMaterialObj.Object);
	}
}

void ACameraVolumeActor::UpdateSidesIndicators()
{
	//Priority
	Text_Indicators[0]->SetText(FText::FromString(FString::FromInt(Priority)));
	Text_Indicators[0]->SetWorldSize(2.f * TextSize);
	Text_Indicators[0]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f)); //Side-scroller
	//Text_Indicators[0]->SetRelativeLocationAndRotation(FVector(0.f, TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f)); //Top-down

//Sides
	for (uint8 i = 1; i <= 18; i = i + 3)
	{
		Text_Indicators[i]->SetWorldSize(TextSize);
		Text_Indicators[i + 1]->SetWorldSize(TextSize);
		Text_Indicators[i + 2]->SetWorldSize(TextSize);

		FSideInfo SideInfo;
		if (i == 1)
		{
			SideInfo = FrontSide;
			Text_Indicators[i]->SetText(Text_Front);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 2.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 1.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y - 0.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				//Side-scroller
				Text_Indicators[i]->SetVisibility(false);
				Text_Indicators[i + 1]->SetVisibility(false);
				Text_Indicators[i + 2]->SetVisibility(false);
				//Top-down
				//Text_Indicators[i]->SetVisibility(true);
				//Text_Indicators[i + 1]->SetVisibility(true);
				//Text_Indicators[i + 2]->SetVisibility(true);
			}
		}
		else if (i == 4)
		{
			SideInfo = BackSide;
			Text_Indicators[i]->SetText(Text_Back);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 0.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 1.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, -VolumeExtent.Y + 2.5f * TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				//Side-scroller
				Text_Indicators[i]->SetVisibility(false);
				Text_Indicators[i + 1]->SetVisibility(false);
				Text_Indicators[i + 2]->SetVisibility(false);
				//Top-down
				//Text_Indicators[i]->SetVisibility(true);
				//Text_Indicators[i + 1]->SetVisibility(true);
				//Text_Indicators[i + 2]->SetVisibility(true);
			}
		}
		else if (i == 7)
		{
			SideInfo = RightSide;
			Text_Indicators[i]->SetText(Text_Right);
			//Side-scroller
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, 0.f), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, VolumeExtent.Y + 5.f, -TextSize), FRotator(0.f, 90.f, 0.f));
			//Top-down
			//Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, -TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			//Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, 0.0f, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			//Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(VolumeExtent.X - 2.f * TextSize, TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
		}
		else if (i == 10)
		{
			SideInfo = LeftSide;
			Text_Indicators[i]->SetText(Text_Left);
			//Side-scroller
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, 0.f), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, VolumeExtent.Y + 5.f, -TextSize), FRotator(0.f, 90.f, 0.f));
			//Top-down
			//Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, -TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			//Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, 0.f, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
			//Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(-VolumeExtent.X + 2.f * TextSize, TextSize, VolumeExtent.Z + 5.f), FRotator(90.f, 90.f, 0.f));
		}
		else if (i == 13)
		{
			SideInfo = TopSide;
			Text_Indicators[i]->SetText(Text_Top);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 0.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 1.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, VolumeExtent.Z - 2.5f * TextSize), FRotator(0.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				//Side-scroller
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
				//Top-down
				//Text_Indicators[i]->SetVisibility(false);
				//Text_Indicators[i + 1]->SetVisibility(false);
				//Text_Indicators[i + 2]->SetVisibility(false);
			}
		}
		else if (i == 16)
		{
			SideInfo = BottomSide;
			Text_Indicators[i]->SetText(Text_Bottom);
			Text_Indicators[i]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 2.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 1]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 1.5f * TextSize), FRotator(0.f, 90.f, 0.f));
			Text_Indicators[i + 2]->SetRelativeLocationAndRotation(FVector(0.f, VolumeExtent.Y + 5.f, -VolumeExtent.Z + 0.5f * TextSize), FRotator(0.f, 90.f, 0.f));

			if (bUse6DOFVolume)
			{
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
			}
			else
			{
				//Side-scroller
				Text_Indicators[i]->SetVisibility(true);
				Text_Indicators[i + 1]->SetVisibility(true);
				Text_Indicators[i + 2]->SetVisibility(true);
				//Top-down
				//Text_Indicators[i]->SetVisibility(false);
				//Text_Indicators[i + 1]->SetVisibility(false);
				//Text_Indicators[i + 2]->SetVisibility(false);
			}
		}

		if (SideInfo.SideType == ESideType::EST_Closed)
		{
			Text_Indicators[i + 1]->SetText(Text_Closed);
			Text_Indicators[i + 1]->SetTextRenderColor(FColor::Red);
		}
		else
		{
			Text_Indicators[i + 1]->SetText(Text_Open);
			Text_Indicators[i + 1]->SetTextRenderColor(FColor::Green);
		}

		switch (SideInfo.SideTransitionType)
		{
		case ESideTransitionType::ESTT_Normal:
			Text_Indicators[i + 2]->SetText(Text_Normal);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Yellow);
			break;
		case ESideTransitionType::ESTT_Smooth:
			Text_Indicators[i + 2]->SetText(Text_Smooth);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Green);
			break;
		case ESideTransitionType::ESTT_Cut:
			Text_Indicators[i + 2]->SetText(Text_Cut);
			Text_Indicators[i + 2]->SetTextRenderColor(FColor::Red);
			break;
		}
	}
}
#endif

//Update with changed property
#if WITH_EDITOR
void ACameraVolumeActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == TEXT("Priority") || TEXT("VolumeExtent")
		|| TEXT("bUseZeroDepthExtent") || TEXT("bUse6DOFVolume")
		|| TEXT("CameraProjectionMode")
		|| TEXT("CameraMobility")
		|| TEXT("bOverrideCameraLocation") || TEXT("CameraLocation")
		|| TEXT("bOverrideCameraRotation") || TEXT("CameraFocalPoint") || TEXT("CameraRoll")
		|| TEXT("bOverrideCameraFieldOfView") || TEXT("CameraFieldOfView")
		|| TEXT("bOverrideCameraOrthoWidth") || TEXT("CameraOrthoWidth")
		|| TEXT("FrontSide") || TEXT("BackSide") || TEXT("RightSide") || TEXT("LeftSide") || TEXT("TopSide") || TEXT("BottomSide")
		|| TEXT("TextSize"))
		UpdateVolume();
}

void ACameraVolumeActor::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyScale(const FVector& DeltaScale, const FVector* PivotLocation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyScale(DeltaScale, PivotLocation, bAltDown, bShiftDown, bCtrlDown);

	FVector CurrentScale = VolumeExtent / VolumeExtentDefault;
	FVector ScaleToApply;

	if (AActor::bUsePercentageBasedScaling)
		ScaleToApply = CurrentScale * (FVector::OneVector + DeltaScale);
	else
		ScaleToApply = CurrentScale + DeltaScale;

	VolumeExtent = VolumeExtentDefault * ScaleToApply;
	UpdateVolume();
}

void ACameraVolumeActor::EditorApplyMirror(const FVector& MirrorScale, const FVector& PivotLocation)
{
	Super::EditorApplyMirror(MirrorScale, PivotLocation);
	SetActorScale3D(FVector::OneVector);
}
#endif

// Runtime setters
void ACameraVolumeActor::SetVolumeExtent(FVector NewVolumeExtent)
{
	VolumeExtent = NewVolumeExtent;
	UpdateVolume();
}

void ACameraVolumeActor::SetUseZeroDepthExtent(bool bNewUseZeroDepthExtent)
{
	bUseZeroDepthExtent = bNewUseZeroDepthExtent;
	UpdateVolume();
}

void ACameraVolumeActor::SetUse6DOFVolume(bool bNewUse6DOFVolume)
{
	bUse6DOFVolume = bNewUse6DOFVolume;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraMobility(ECameraMobility NewCameraMobility)
{
	CameraMobility = NewCameraMobility;
	UpdateVolume();
}

void ACameraVolumeActor::SetOverrideCameraLocation(bool bNewOverrideCameraLocation)
{
	bOverrideCameraLocation = bNewOverrideCameraLocation;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraLocation(FVector NewCameraLocation)
{
	CameraLocation = NewCameraLocation;
	UpdateVolume();
}

void ACameraVolumeActor::SetOverrideCameraRotation(bool bNewOverrideCameraRotation)
{
	bOverrideCameraRotation = bNewOverrideCameraRotation;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraFocalPoint(FVector NewCameraFocalPoint)
{
	CameraFocalPoint = NewCameraFocalPoint;
	UpdateVolume();
}

void ACameraVolumeActor::SetCameraRoll(float NewCameraRoll)
{
	CameraRoll = NewCameraRoll;
	UpdateVolume();
}

void ACameraVolumeActor::SetRightSide(FSideInfo NewRightSide)
{
	RightSide = NewRightSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetLeftSide(FSideInfo NewLeftSide)
{
	LeftSide = NewLeftSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetTopSide(FSideInfo NewTopSide)
{
	TopSide = NewTopSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetBottomSide(FSideInfo NewBottomSide)
{
	BottomSide = NewBottomSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetFrontSide(FSideInfo NewFrontSide)
{
	FrontSide = NewFrontSide;
	CalculateVolumeExtents();
}

void ACameraVolumeActor::SetBackSide(FSideInfo NewBackSide)
{
	BackSide = NewBackSide;
	CalculateVolumeExtents();
}