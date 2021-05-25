// Copyright Bruno Silva. All rights reserved.


#include "ProceduralGenerator.h"
#include "DrawDebugHelpers.h"

void FQuad2D::DebugDraw(UWorld* InWorld, FColor DrawColor, float Height) const
{
	DrawDebugLine(InWorld, FVector(A, Height), FVector(B, Height), DrawColor, true, -1.0f, 0, 5.0f);
	DrawDebugLine(InWorld, FVector(B, Height), FVector(C, Height), DrawColor, true, -1.0f, 0, 5.0f);
	DrawDebugLine(InWorld, FVector(C, Height), FVector(D, Height), DrawColor, true, -1.0f, 0, 5.0f);
	DrawDebugLine(InWorld, FVector(D, Height), FVector(A, Height), DrawColor, true, -1.0f, 0, 5.0f);
}

bool FQuad2D::Equals(const FQuad2D& Other) const
{
	const bool DoesAMatch = A.Equals(Other.A, 1e-6f);
	const bool DoesBMatch = B.Equals(Other.B, 1e-6f);
	const bool DoesCMatch = C.Equals(Other.C, 1e-6f);
	const bool DoesDMatch = D.Equals(Other.D, 1e-6f);
	return DoesAMatch && DoesBMatch && DoesCMatch && DoesDMatch;
}

void UGeneratorLibrary::DrawQuad2D(UObject* WorldContext, FQuad2D QuadToDraw, FLinearColor LineColor, float Height)
{
	if (WorldContext)
	{
		QuadToDraw.DebugDraw(WorldContext->GetWorld(), LineColor.ToFColor(true), Height);
	}
}

void UGeneratorLibrary::DivideQuad2D(const FQuad2D& InQuad, const float Fraction, const bool bUseADAxis, TArray<FQuad2D>& OutResult)
{
	FQuad2D NewQuad1;
	FQuad2D NewQuad2;
	if (bUseADAxis)
	{
		NewQuad1 = FQuad2D(
			InQuad.A,
			InQuad.B,
			InQuad.B + (InQuad.GetBC() * Fraction),
			InQuad.A - (InQuad.GetDA() * Fraction));
		NewQuad2 = FQuad2D(
			InQuad.A - (InQuad.GetDA() * Fraction),
			InQuad.B + (InQuad.GetBC() * Fraction),
			InQuad.C,
			InQuad.D);
	}
	else
	{
		NewQuad1 = FQuad2D(
			InQuad.A,
			InQuad.A + (InQuad.GetAB() * Fraction),
			InQuad.D - (InQuad.GetCD() * Fraction),
			InQuad.D);
		NewQuad2 = FQuad2D(
			InQuad.A + (InQuad.GetAB() * Fraction),
			InQuad.B,
			InQuad.C,
			InQuad.D - (InQuad.GetCD() * Fraction));
	}
	OutResult.Add(NewQuad1);
	OutResult.Add(NewQuad2);
}

void UGeneratorLibrary::DivideQuad2DMultiple(const FQuad2D& InQuad, TArray<float> Fractions, const bool bUseADAxis, TArray<FQuad2D>& OutResult)
{
	FQuad2D NewQuad1;
	FQuad2D NewQuad2 = InQuad;
	float Fraction = 0.0f;
	for (const float Value : Fractions)
	{
		Fraction += Value;
		if (Fraction >= 1.0f)
		{
			OutResult.Add(NewQuad2);
			return;
		}
		if (bUseADAxis)
		{
			NewQuad1 = FQuad2D(
				NewQuad2.A,
				NewQuad2.B,
				InQuad.B + (InQuad.GetBC() * Fraction),
				InQuad.A - (InQuad.GetDA() * Fraction));
			NewQuad2 = FQuad2D(
				InQuad.A - (InQuad.GetDA() * Fraction),
				InQuad.B + (InQuad.GetBC() * Fraction),
				InQuad.C,
				InQuad.D);
		}
		else
		{
			NewQuad1 = FQuad2D(
				NewQuad2.A,
				InQuad.A + (InQuad.GetAB() * Fraction),
				InQuad.D - (InQuad.GetCD() * Fraction),
				NewQuad2.D);
			NewQuad2 = FQuad2D(
				InQuad.A + (InQuad.GetAB() * Fraction),
				InQuad.B,
				InQuad.C,
				InQuad.D - (InQuad.GetCD() * Fraction));
		}
		OutResult.Add(NewQuad1);
	}
	OutResult.Add(NewQuad2);
}

FQuad2D UGeneratorLibrary::ResizeQuad2D(const FQuad2D& InQuad, const float Delta)
{
	FQuad2D ResizedQuad = InQuad;

	// Scale the VectorQuad by moving each vector towards or
	// away from their respective adjacent vectors.

	ResizedQuad.A += (InQuad.GetDA().GetSafeNormal(1e-6f) * Delta
		+ InQuad.GetAB().GetSafeNormal(1e-6f) * Delta * -1.0f);
	ResizedQuad.B += (InQuad.GetAB().GetSafeNormal(1e-6f) * Delta
		+ InQuad.GetBC().GetSafeNormal(1e-6f) * Delta * -1.0f);
	ResizedQuad.C += (InQuad.GetBC().GetSafeNormal(1e-6f) * Delta
		+ InQuad.GetCD().GetSafeNormal(1e-6f) * Delta * -1.0f);
	ResizedQuad.D += (InQuad.GetCD().GetSafeNormal(1e-6f) * Delta
		+ InQuad.GetDA().GetSafeNormal(1e-6f) * Delta * -1.0f);

	return ResizedQuad;
}

void UGeneratorLibrary::ResizeQuad2DRef(FQuad2D& InOutQuad, const float Delta)
{
	InOutQuad.A += (InOutQuad.GetDA().GetSafeNormal(1e-6f) * Delta
		+ InOutQuad.GetAB().GetSafeNormal(1e-6f) * Delta * -1.0f);
	InOutQuad.B += (InOutQuad.GetAB().GetSafeNormal(1e-6f) * Delta
		+ InOutQuad.GetBC().GetSafeNormal(1e-6f) * Delta * -1.0f);
	InOutQuad.C += (InOutQuad.GetBC().GetSafeNormal(1e-6f) * Delta
		+ InOutQuad.GetCD().GetSafeNormal(1e-6f) * Delta * -1.0f);
	InOutQuad.D += (InOutQuad.GetCD().GetSafeNormal(1e-6f) * Delta
		+ InOutQuad.GetDA().GetSafeNormal(1e-6f) * Delta * -1.0f);
}
