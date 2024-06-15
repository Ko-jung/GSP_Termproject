#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

#include "GameMgr.h"
#include "Actor.h"

#include "framework.h"

#include "../../Common/protocol.h"
#include "../../Common/EnumDef.h"
#include "../../Common/GameUtil.h"
#include "Define.h"

void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

GameMgr::GameMgr() :
	ElapsedTime(0.f),
	SerialNum(-1),
	RemainDataLen(0)
{
	OwnActor = std::make_shared<Actor>();

	WorldImageTile.Load(TEXT("Image/Map/MapImage.png"));
	LoadBoard();

	PrevTime = std::chrono::system_clock::now();
}

GameMgr::~GameMgr()
{
	closesocket(ServerSocket);
	WSACleanup();
}

bool GameMgr::InitSocket()
{
	WSADATA WSAData;
	int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret)
	{
		return false;
	}
	return true;
}

bool GameMgr::Connect(const char* ip)
{
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(GAMESERVERPORT);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	int ret = connect(ServerSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (ret != 0)
	{
		error_display("connect Error: ", WSAGetLastError());
		return false;
	}

	//NetworkThread = std::thread(&GameMgr::Recv, this);
	Recv();

	return true;
}

void GameMgr::Draw(HDC& memdc)
{
	DrawBoard(memdc);

	for (const auto& a : OtherActors)
	{
		a.second->Draw(memdc);
	}

	//Recv();
	OwnActor->Draw(memdc);
}

void GameMgr::Update()
{
	float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - PrevTime).count() / 1000.f;
	PrevTime = std::chrono::system_clock::now();

	OwnActor->Update(elapsedTime);

	SendPosition();

	std::cout << OwnActor->GetLocation().X << ", " << OwnActor->GetLocation().Y << std::endl;
}

void GameMgr::ProcessUpInput(WPARAM wParam)
{	
	OwnActor->ProcessUpInput(wParam);
}

void GameMgr::ProcessDownInput(WPARAM wParam)
{
	OwnActor->ProcessDownInput(wParam);
}

void GameMgr::LoadBoard()
{
	CImage WorldImage;
	WorldImage.Load(TEXT("../../Common/Image/MiniWorldMapTile.png"));
	//WorldImage.Load(TEXT("../../Common/Image/WorldMapTile.png"));

	int Width = WorldImage.GetWidth();
	int Height = WorldImage.GetHeight();

	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			COLORREF PixelColor = WorldImage.GetPixel(i, j);
			int Red = GetRValue(PixelColor);
			int Green = GetGValue(PixelColor);
			int Blue = GetBValue(PixelColor);

			if (Red == 0 && Green == 0 && Blue == 0)
			{
				WorldMap[i][j] = (BYTE)MAP_INFO::WALLS_BLOCK;
			}
			else if (Red == 255 && Green == 0 && Blue == 0)
			{

			}
			else if (Red == 0 && Green == 255 && Blue == 0)
			{
				WorldMap[i][j] = (BYTE)MAP_INFO::GROUND_EMPTY;
			}
			else if (Red == 0 && Green == 0 && Blue == 255)
			{
				WorldMap[i][j] = (BYTE)MAP_INFO::GROUND_EMPTY;
			}
		}
	}
}

void GameMgr::DrawBoard(HDC& memdc)
{
	POSITION pos = OwnActor->GetLocation();
	//std::cout << X << ", " << Y << std::endl;

	for (int y = 0; y < BOARDSIZE; y++)
	{
		for (int x = 0; x < BOARDSIZE; x++)
		{
			int DrawX = (int)pos.X + x - BOARDSIZE / 2;
			int DrawY = (int)pos.Y + y - BOARDSIZE / 2;

			if (DrawX < 0 || DrawX >= W_WIDTH || DrawY < 0 || DrawY >= W_WIDTH) continue;

			int BoardDrawSize = WINWIDTH / BOARDSIZE;
			RECT TileDest{ x * BoardDrawSize, y * BoardDrawSize, (x + 1) * BoardDrawSize, (y + 1) * BoardDrawSize };
			switch (WorldMap[DrawY][DrawX])
			{
			case (BYTE)MAP_INFO::WALLS_BLOCK:
			{
				// 109, 188
				// 132, 211
				RECT Tile{ 109,188,132,211 };
				WorldImageTile.Draw(memdc, TileDest, Tile);
				break;
			}
			case (BYTE)MAP_INFO::GROUND_EMPTY:
			{
				// 334, 188
				// 357, 211
				RECT Tile{ 334,188,357,211 };
				WorldImageTile.Draw(memdc, TileDest, Tile);
				break;
			}
			default:
				break;
			}
		}
	}
}

void GameMgr::SendPosition()
{
	// POSITION Pos = OwnActor->GetLocation();


	CS_8DIRECT_MOVE_PACKET CMP;
	CMP.bitDirection = OwnActor->GetKeyInputInfo();
	CMP.direction = (char)OwnActor->GetDirection();

	Send(&CMP);
}

void GameMgr::SetOwnActorID(const char* ID)
{
	OwnActor->SetName(ID);
}

void GameMgr::error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L" 에러 " << lpMsgBuf << std::endl;
	//while (true);
	// 디버깅 용
	LocalFree(lpMsgBuf);
}

//void GameMgr::Recv()
//{
//    DWORD recv_byte;
//    DWORD recv_flag = 0;
//    char recv_buf[CHAT_SIZE];
//    WSABUF mybuf_r;
//    mybuf_r.buf = recv_buf;
//    mybuf_r.len = CHAT_SIZE;
//
//    WSARecv(ServerSocket, &mybuf_r, 1, &recv_byte, &recv_flag, 0, 0);
//    ProcessRecv(recv_buf);
//}

void GameMgr::ProcessRecv(PACKET* packet)
{
	switch (packet->type)
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* SLIP = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		SerialNum = SLIP->id;

		OwnActor->ProcessLogin(SLIP);
		break;
	}
	case SC_LOGIN_FAIL:
		break;
	//case SC_ADD_OBJECT:
	//	break;
	case SC_REMOVE_OBJECT:
		break;
	case SC_MOVE_OBJECT:
		break;
	case SC_CHAT:
		break;
	case SC_STAT_CHANGE:
		break;
	case SC_8DIRECT_MOVE_OBJECT:
	{
		SC_8DIRECT_MOVE_OBJECT_PACKET* SDMOP = reinterpret_cast<SC_8DIRECT_MOVE_OBJECT_PACKET*>(packet);
	
		if (SDMOP->id == SerialNum)
		{
			OwnActor->ProcessMove(SDMOP);
		}
		else
		{
			Actor* TargetActor = nullptr;
			if (OtherActors.find(SDMOP->id) == OtherActors.end())
			{
				std::cout << "Get SC_8DIRECT_MOVE_OBJECT_PACKET but id:" << SDMOP->id << " is Cant Find!" << std::endl;
				return;
			}

			OtherActors[SDMOP->id]->ProcessMove(SDMOP);
		}
		
		break;
	}
	default:
		break;
	}
}

void GameMgr::Send(PACKET* p)
{
	OverExpansion* exp = new OverExpansion((char*)p);

	int ret = WSASend(ServerSocket, &exp->_wsabuf, 1, nullptr, 0, &exp->_over, ::send_callback);
	if (ret != 0 && ret != WSA_IO_PENDING)
	{
		error_display("WSASend Error: ", WSAGetLastError());
	}
}

void GameMgr::Recv()
{
	DWORD RecvFlag = 0;

	OverExpansion* Exp = new OverExpansion;
	ZeroMemory(&Exp->_over, sizeof(Exp->_over));
	Exp->_wsabuf.buf = reinterpret_cast<char*>(Exp->_send_buf + RemainDataLen);
	Exp->_wsabuf.len = sizeof(Exp->_send_buf) - RemainDataLen;

	int ret = WSARecv(ServerSocket, &Exp->_wsabuf, 1, nullptr, &RecvFlag, &Exp->_over, ::recv_callback);
	if (ret != 0)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		error_display("WSARecv Error: ", WSAGetLastError());
	}
}

void GameMgr::recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	if (r_size == 0)
	{
		std::cout << "Maybe Server CLOSE" << std::endl;
		exit(0);
		return;
	}

	OverExpansion* exp = reinterpret_cast<OverExpansion*>(p_wsaover);

	int RemainData = r_size + RemainDataLen;
	char* Buf = exp->_send_buf;

	while (RemainData > 0)
	{
		PACKET* packet = reinterpret_cast<PACKET*>(Buf);
		if (RemainData >= packet->size)
		{
			ProcessRecv(packet);
			Buf += packet->size;
			RemainData -= packet->size;
		}
		else
			break;
	}
	RemainDataLen = RemainData;

	if (RemainData > 0)
		memmove(exp->_send_buf, Buf, RemainData);

	Recv();
	//Send();
}

void GameMgr::send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	delete p_wsaover;
}
