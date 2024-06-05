#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

#include "GameMgr.h"
#include "Actor.h"

#include "framework.h"
#include "../../Common/protocol.h"
#include <iostream>

#include "../../Common/EnumDef.h"
#include "Define.h"

void CALLBACK recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);
void CALLBACK send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag);

GameMgr::GameMgr()
{
	OwnActor = std::make_shared<Actor>();
	SerialNum = -1;

	KeyInputInfo.assign(4, false);

	WorldImageTile.Load(TEXT("Image/Map/MapImage.png"));
	LoadBoard();
}

GameMgr::~GameMgr()
{
	closesocket(s_socket);
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
	s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(GAMESERVERPORT);
	inet_pton(AF_INET, ip, &server_addr.sin_addr);
	int ret = connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	if (ret != 0)
	{
		error_display("connect Error: ", WSAGetLastError());
		return false;
	}

	//NetworkThread = std::thread(&GameMgr::Recv, this);

	return true;
}

void GameMgr::Draw(HDC& memdc)
{
	DrawBoard(memdc);

	for (const auto& a : OtherActor)
	{
		a.second->Draw(memdc);
	}

	//Recv();
	OwnActor->Draw(memdc);
}

void GameMgr::Update(float elapsedTime)
{
	OwnActor->Update(elapsedTime);
}

void GameMgr::ProcessUpInput(WPARAM wParam)
{
	// Check Input Move Key
	int Key = -1;

	switch (wParam)
	{
	case 'w':
	case 'W':
	case VK_UP:
	{
		Key = (int)KEY_INFO::UP;
		break;
	}
	case 'a':
	case 'A':
	case VK_LEFT:
	{
		Key = (int)KEY_INFO::LEFT;
		break;
	}
	case 's':
	case 'S':
	case VK_DOWN:
	{
		Key = (int)KEY_INFO::DOWN;
		break;
	}
	case 'd':
	case 'D':
	case VK_RIGHT:
	{
		Key = (int)KEY_INFO::RIGHT;
		break;
	}
	default:
		break;
	}

	// Process Input
	if (Key != -1)
	{
		KeyInputInfo[Key] = false;
	}
}

void GameMgr::ProcessDownInput(WPARAM wParam)
{
	// Check Input Move Key
	int Key = -1;

	switch (wParam)
	{
	case 'w':
	case 'W':
	case VK_UP:
	{
		Key = (int)KEY_INFO::UP;
		break;
	}
	case 'a':
	case 'A':
	case VK_LEFT:
	{
		Key = (int)KEY_INFO::LEFT;
		break;
	}
	case 's':
	case 'S':
	case VK_DOWN:
	{
		Key = (int)KEY_INFO::DOWN;
		break;
	}
	case 'd':
	case 'D':
	case VK_RIGHT:
	{
		Key = (int)KEY_INFO::RIGHT;
		break;
	}
	default:
		break;
	}

	// Process Input
	if (Key != -1)
	{
		KeyInputInfo[Key] = true;
	}
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
	float X, Y;
	OwnActor->GetLocation(X, Y);
	std::cout << X << ", " << Y << std::endl;

	for (int y = 0; y < BOARDSIZE; y++)
	{
		for (int x = 0; x < BOARDSIZE; x++)
		{
			int DrawX = X + x - BOARDSIZE / 2;
			int DrawY = Y + y - BOARDSIZE / 2;

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
//    WSARecv(s_socket, &mybuf_r, 1, &recv_byte, &recv_flag, 0, 0);
//    ProcessRecv(recv_buf);
//}

void GameMgr::ProcessRecv(char* recv_buf)
{
	//switch (op)
	//{
	//case COMP_OP::OP_ACTORLOCATION:
	//{
	//	PActorLocation PAL;
	//	memcpy(&PAL, recv_buf, sizeof(PActorLocation));
	//	if (SerialNum == PAL.SenderId)
	//	{
	//		// 서버에서 보내온 좌표로 말을 이동시킨다.
	//		OwnActor->SetLocation(PAL.Location);
	//	}
	//	else
	//	{
	//		// 서버에서 보내온 좌표로 말을 이동시킨다.
	//		OtherActor[PAL.SenderId]->SetLocation(PAL.Location);
	//	}
	//	break;
	//}
	//case COMP_OP::OP_SETSERIALNUM:
	//{
	//	PSetPlayerSerialNum PSPSN;
	//	memcpy(&PSPSN, recv_buf, sizeof(PSPSN));
	//	SerialNum = PSPSN.SenderId;
	//	break;
	//}
	//case COMP_OP::OP_JOINNEWPLAYER:
	//{
	//	PJoinNewPlayer PJNP;
	//	memcpy(&PJNP, recv_buf, sizeof(PJNP));
	//	if (SerialNum == -1)
	//	{
	//		SerialNum = PJNP.SenderId;
	//	}
	//	else
	//	{
	//		OtherActor.try_emplace(PJNP.SenderId, std::make_shared<Actor>(false));
	//		LogUtil::print("New Player Join!");
	//	}
	//	break;
	//}
	//case COMP_OP::OP_DISCONNECTPLAYER:
	//{
	//	PDisconnectPlayer PDP;
	//	memcpy(&PDP, recv_buf, sizeof(PDP));
	//	OtherActor.erase(PDP.SenderId);
	//
	//	char msg[100];
	//	sprintf_s(msg, "%d Client Disconnect", 100);
	//	LogUtil::print(msg);
	//	break;
	//}
	//case COMP_OP::OP_LOADALLCLIENT:
	//{
	//	PLoadAllClient PLA;
	//	memcpy(&PLA, recv_buf, sizeof(PLA));
	//
	//	int i = 0;
	//	while (true)
	//	{
	//		int serial = PLA.SerialIDs[i];
	//
	//		if (serial == (BYTE)(-1))
	//			break;
	//		if (serial == SerialNum)
	//		{
	//			i++;
	//			continue;
	//		}
	//
	//		OtherActor.try_emplace(serial, std::make_shared<Actor>(false));
	//		OtherActor[serial]->SetLocation(PLA.Locations[i]);
	//		i++;
	//	} 
	//
	//	break;
	//}
	//default:
	//	break;
	//}
}

void GameMgr::Send(PACKET* p, int packetSize)
{
	DWORD sent_byte;
	WSABUF mybuf;
	mybuf.buf = send_buf;
	mybuf.len = packetSize;// static_cast<ULONG>(strlen(send_buf)) + 1;
	memcpy(mybuf.buf, p, packetSize);
	//mybuf.buf = send_buf;

	ZeroMemory(&wsaover, sizeof(wsaover));
	int ret = WSASend(s_socket, &mybuf, 1, nullptr, 0, &wsaover, ::send_callback);
	if (ret != 0)
	{
		error_display("WSASend Error: ", WSAGetLastError());
	}
}

void GameMgr::Recv()
{
	DWORD recv_flag = 0;
	mybuf[0].buf = recv_buf;
	mybuf[0].len = CHAT_SIZE;

	ZeroMemory(&wsaover, sizeof(wsaover));
	WSARecv(s_socket, mybuf, 1, nullptr, &recv_flag, &wsaover, ::recv_callback);
}

void GameMgr::recv_callback(DWORD errors, DWORD r_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	if (r_size == 0)
	{
		//g_bLogout = true;
		std::cout << "Maybe Server CLOSE" << std::endl;
		exit(0);
		return;
	}

	ProcessRecv(recv_buf);

	Recv();
	//Send();
}

void GameMgr::send_callback(DWORD errors, DWORD transfer_size, LPWSAOVERLAPPED p_wsaover, DWORD recv_flag)
{
	//mybuf[0].buf = recv_buf;
	//mybuf[0].len = CHAT_SIZE;
	//
	//ZeroMemory(&wsaover, sizeof(wsaover));
	//WSARecv(s_socket, mybuf, 1, nullptr, &recv_flag, &wsaover, ::recv_callback);
}
