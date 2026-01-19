#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "Common/UdpSocketReceiver.h"
#include "UdpReceiverActor.generated.h"

UCLASS()
class UDP_TEST_API AUdpReceiverActor : public AActor
{
    GENERATED_BODY()

public:
    AUdpReceiverActor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // Last received UDP message (for debug / parsing)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UDP :)")
    FString LastMessage;

    // Editable port, visible in Details panel
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PORT :)", meta = (DisplayName = "Port :)"))
    int32 ListenPort = 9000;

protected:
    // Low-level UDP objects
    FSocket* ListenSocket = nullptr;
    FUdpSocketReceiver* UDPReceiver = nullptr;

    // Internal helpers
    void StartUDPListener(const FString& ListenIP = TEXT("0.0.0.0"), int32 InListenPort = 9000);
    void StopUDPListener();
    void OnDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint);
};
