// Copyright Bruno Silva. All rights reserved.

#pragma once

#include "Portfolio\Portfolio.h"
#include "ProceduralGenerator.generated.h"


USTRUCT(BlueprintType)
struct FQuad2D
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quad")
	FVector2D A;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quad")
	FVector2D B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quad")
	FVector2D C;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quad")
	FVector2D D;

	FQuad2D() {}

	FQuad2D(FVector2D NewA, FVector2D NewB, FVector2D NewC, FVector2D NewD)
	{
		A = NewA;
		B = NewB;
		C = NewC;
		D = NewD;
	}

	FQuad2D(const FQuad2D& Other)
	{
		A = Other.A;
		B = Other.B;
		C = Other.C;
		D = Other.D;
	}

public:

	FVector2D GetAB() const { return (B - A); };
	FVector2D GetBC() const { return (C - B); };
	FVector2D GetCD() const { return (D - C); };
	FVector2D GetDA() const { return (A - D); };

	/** Draw the vector quad in the world, for debug purposes. */
	void DebugDraw(UWorld* InWorld, FColor DrawColor, float Height) const;

	bool Equals(const FQuad2D& Other) const;

public:

	bool operator==(const FQuad2D& OtherQuad) const
	{
		const bool bIsEqual = Equals(OtherQuad);
		return bIsEqual;
	}

	bool operator!=(const FQuad2D& OtherQuad) const
	{
		const bool bIsEqual = Equals(OtherQuad);
		return !bIsEqual;
	}

	FQuad2D operator+(const FVector2D& Offset) const
	{
		FQuad2D Result;
		Result.A = A + Offset;
		Result.B = B + Offset;
		Result.C = C + Offset;
		Result.D = D + Offset;
		return Result;
	}

	FQuad2D operator-(const FVector2D& Offset) const
	{
		FQuad2D Result;
		Result.A = A - Offset;
		Result.B = B - Offset;
		Result.C = C - Offset;
		Result.D = D - Offset;
		return Result;
	}
};

UCLASS()
class PORTFOLIO_API UGeneratorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Draw a VectorQuad in the world. */
	UFUNCTION(BlueprintCallable, Category = "VectorQuad", meta = (DisplayName = "Draw Quad2D"))
	static void DrawQuad2D(UObject* WorldContext, FQuad2D QuadToDraw, FLinearColor LineColor, float Height);

	/** Divide one quad into two smaller quads. */
	UFUNCTION(BlueprintCallable, Category = "VectorQuad", meta = (DisplayName = "Divide Quad2D"))
	static void DivideQuad2D(const FQuad2D& InQuad, const float Fraction, const bool bUseADAxis, TArray<FQuad2D>& OutResult);

	/** Divide one quad into several smaller quads. */
	UFUNCTION(BlueprintCallable, Category = "VectorQuad", meta = (DisplayName = "Divide Quad2D Multiple"))
	static void DivideQuad2DMultiple(const FQuad2D& InQuad, TArray<float> Fractions, const bool bUseADAxis, TArray<FQuad2D>& OutResult);

	/** Scale the given VectorQuad linearly. */
	UFUNCTION(BlueprintCallable, Category = "VectorQuad", meta = (DisplayName = "Resize Quad2D"))
	static FQuad2D ResizeQuad2D(const FQuad2D& InQuad, const float Delta);

	/** Scale the given VectorQuad linearly. */
	UFUNCTION(BlueprintCallable, Category = "VectorQuad", meta = (DisplayName = "Resize Quad2D Ref"))
	static void ResizeQuad2DRef(UPARAM(ref) FQuad2D& InOutQuad, const float Delta);
};