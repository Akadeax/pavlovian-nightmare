// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenu::MenuSetup(int32 SessionNumPublicConnections, FString SessionMatchType, FString SessionLobbyPath)
{
	NumPublicConnections = SessionNumPublicConnections;
	MatchType = SessionMatchType;
	LobbyPath = FString::Printf(TEXT("%s?listen"), *SessionLobbyPath);
	
	const UGameInstance* GameInstance{ GetGameInstance() };
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance is nullptr!"));
		return;
	}
	
	MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is nullptr!"));
		return;
	}

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	const UWorld* World{ GetWorld() };
	if(World == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is nullptr!"));
		return;
	}

	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	if(PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is nullptr!"));
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(true);

	MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
	MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
	MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

    MultiplayerSessionsSubsystem->WaitForInviteAccept();
	MultiplayerSessionsSubsystem->MultiplayerOnSessionUserInviteAccepted.AddUObject(this, &ThisClass::OnUserInviteAccepted);
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 15.f, FColor::Green,
				FString("Session created successfully!")
			);
		}
		UWorld* World{ GetWorld() };
		if (World == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("World is nullptr!"));
			return;
		}
		
		World->ServerTravel(LobbyPath);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString("Failed to create session!"));
		}
		
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is nullptr!"));
		return;
	}
	
	for (const FOnlineSessionSearchResult& Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if (SettingsValue != MatchType) continue;

		MultiplayerSessionsSubsystem->JoinSession(Result);
		return;
	}

	if (!bWasSuccessful || SessionResults.IsEmpty())
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSubsystem* OnlineSubsystem{ IOnlineSubsystem::Get() };
	if(OnlineSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystem is nullptr!"));
		return;
	}

	const IOnlineSessionPtr SessionInterface{ OnlineSubsystem->GetSessionInterface() };
	if(SessionInterface == nullptr || !SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SessionInterface is nullptr or not valid!"));
		return;
	}

	FString Address;
	SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
	
	APlayerController* PlayerController{ GetGameInstance()->GetFirstLocalPlayerController() };
	if(PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is nullptr!"));
		return;
	}
	
	PlayerController->ClientTravel(Address, TRAVEL_Absolute);

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::OnUserInviteAccepted(const FOnlineSessionSearchResult& InviteResult, const bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		HostButton->SetIsEnabled(false);
		JoinButton->SetIsEnabled(false);
		MultiplayerSessionsSubsystem->JoinSession(InviteResult);
	}
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString("Host button clicked"));
	}

	if (MultiplayerSessionsSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is nullptr!"));
		return;
	}
	
	MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);

	if(MultiplayerSessionsSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem is nullptr!"));
		return;
	}
	MultiplayerSessionsSubsystem->FindSessions(10'000);
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	
	const UWorld* World{ GetWorld() };
	if (World == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("World is nullptr!"));
		return;
	}
	
	APlayerController* PlayerController{ World->GetFirstPlayerController() };
	if(PlayerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController is nullptr!"));
		return;
	}

	const FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->SetShowMouseCursor(false);
}
