#include "UdpReceiverActor.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Common/UdpSocketBuilder.h"

AUdpReceiverActor::AUdpReceiverActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ListenSocket = nullptr;
    UDPReceiver = nullptr;
}

void AUdpReceiverActor::BeginPlay()
{
    Super::BeginPlay();

    // Listen on all interfaces, using the configurable ListenPort
    StartUDPListener(TEXT("0.0.0.0"), ListenPort);
}

void AUdpReceiverActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopUDPListener();
    Super::EndPlay(EndPlayReason);
}

void AUdpReceiverActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Example: expect "yaw,pitch,roll" as CSV from phone
    if (!LastMessage.IsEmpty())
    {
        TArray<FString> Parts;
        LastMessage.ParseIntoArray(Parts, TEXT(","), true);

        if (Parts.Num() >= 3)
        {
            const float Yaw = FCString::Atof(*Parts[0]);
            const float Pitch = FCString::Atof(*Parts[1]);
            const float Roll = FCString::Atof(*Parts[2]);

            SetActorRotation(FRotator(Pitch, Yaw, Roll));
        }
    }
}

void AUdpReceiverActor::StartUDPListener(const FString& ListenIP, int32 InListenPort)
{
    StopUDPListener(); // clean up if already running

    FIPv4Address Addr;
    if (!FIPv4Address::Parse(ListenIP, Addr))
    {
        UE_LOG(LogTemp, Error, TEXT("UDP: Invalid IP address: %s"), *ListenIP);
        return;
    }

    FIPv4Endpoint Endpoint(Addr, InListenPort);

    ListenSocket = FUdpSocketBuilder(TEXT("UDP_Listener_Socket"))
        .AsNonBlocking()
        .AsReusable()
        .BoundToEndpoint(Endpoint)
        .WithReceiveBufferSize(2 * 1024 * 1024);

    if (!ListenSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("UDP: Failed to create socket on %s:%d"), *ListenIP, InListenPort);
        return;
    }

    UDPReceiver = new FUdpSocketReceiver(
        ListenSocket,
        FTimespan::FromMilliseconds(1),
        TEXT("UDP_Receiver_Thread")
    );

    UDPReceiver->OnDataReceived().BindUObject(this, &AUdpReceiverActor::OnDataReceived);
    UDPReceiver->Start();

    UE_LOG(LogTemp, Log, TEXT("UDP: Listening on %s:%d"), *ListenIP, InListenPort);
}

void AUdpReceiverActor::StopUDPListener()
{
    if (UDPReceiver)
    {
        UDPReceiver->Stop();
        delete UDPReceiver;
        UDPReceiver = nullptr;
    }

    if (ListenSocket)
    {
        ListenSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenSocket);
        ListenSocket = nullptr;
    }
}

void AUdpReceiverActor::OnDataReceived(const FArrayReaderPtr& Data, const FIPv4Endpoint& Endpoint)
{
    const int32 Size = Data->Num();
    const char* RawData = reinterpret_cast<const char*>(Data->GetData());

    FString Message = FString(Size, ANSI_TO_TCHAR(RawData));
    Message.TrimEndInline();

    LastMessage = Message;

    UE_LOG(LogTemp, Log, TEXT("UDP from %s: %s"), *Endpoint.ToString(), *Message);
}
