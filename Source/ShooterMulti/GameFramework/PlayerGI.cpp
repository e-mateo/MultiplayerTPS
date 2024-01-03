#include "PlayerGI.h"
#include "ShooterPS.h"

UPlayerGI::UPlayerGI(const FObjectInitializer& ObjInit) : Super(ObjInit)
{

}

FPlayerInfo UPlayerGI::GetUserInfo()
{
	return UserInfo;
}

void UPlayerGI::SetUserInfo(int32 InTeamNum, const FString& InUserName)
{
	UserInfo.TeamNum = InTeamNum;
	UserInfo.UserName = InUserName;
}

void UPlayerGI::SetUsername(const FString& InUserName)
{
	UserInfo.UserName = InUserName;
}

void UPlayerGI::SetTeamNum(int32 InTeamNum)
{
	UserInfo.TeamNum = InTeamNum;
}