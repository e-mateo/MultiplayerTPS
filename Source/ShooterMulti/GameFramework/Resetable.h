#pragma once

#include "UObject/Interface.h"
#include "Resetable.generated.h"

UINTERFACE(MinimalAPI)
class UResetable : public UInterface
{
	GENERATED_BODY()
};

class SHOOTERMULTI_API IResetable
{
	GENERATED_BODY()

public:
	
	// reset when game ends and victory pops up
	UFUNCTION(Category = "Resetable")
	virtual void Reset() = 0;
};