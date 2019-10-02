#include <Windows.h>
#include <Psapi.h>
#include "Memory.h"
#include "Globals.h"

//All offset variables hardcoded
//namespace Jubs
//namespace MPGH
namespace Offset {
	DWORD_PTR GameManager = 0x473A3D0;
	DWORD_PTR EntityList = 0xC0;

	DWORD_PTR Entity = 0x0008;
	DWORD_PTR EntityRef = 0x20;

	DWORD_PTR EntityInfo = 0x18;
	DWORD_PTR MainComponent = 0xB8;
	DWORD_PTR ChildComponent = 0x8;
	DWORD_PTR Health = 0x108;

	DWORD_PTR PlayerInfo = 0x270;
	DWORD_PTR PlayerInfoDeref = 0x0;
	DWORD_PTR PlayerTeamId = 0x140;
	DWORD_PTR PlayerName = 0x158;

	DWORD_PTR FeetPosition = 0x190;
	DWORD_PTR HeadPosition = 0x130;

	DWORD_PTR weaponComp = 0x38;
	DWORD_PTR WeaponProcessor = 0xF0;
	DWORD_PTR Weapon = 0x0;
	DWORD_PTR WeaponInfo = 0x100;
	DWORD_PTR Spread = 0x2B0;		//float
	DWORD_PTR Recoil = 0x2E8;		//float
	DWORD_PTR Recoil2 = 0x354;		//float
	DWORD_PTR Recoil3 = 0x304;		//Vector4
	DWORD_PTR AdsRecoil = 0x330;	//Vector2

	DWORD_PTR Renderer = 0x46F0800;
	DWORD_PTR GameRenderer = 0x0;
	DWORD_PTR EngineLink = 0xd8;
	DWORD_PTR Engine = 0x218;
	DWORD_PTR Camera = 0x38;

	DWORD_PTR ViewTranslastion = 0x1A0;
	DWORD_PTR ViewRight = 0x170;
	DWORD_PTR ViewUp = 0x180;
	DWORD_PTR ViewForward = 0x190;
	DWORD_PTR FOVX = 0x1B0;
	DWORD_PTR FOVY = 0x1C4;
}

Mem::Mem() {};

BOOL Mem::SetBaseAddress() {
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;

	if (EnumProcessModules(Global::GameHandle, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(Global::GameHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::wstring wstrModName = szModName;
				std::wstring wstrModContain = L"RainbowSix.exe";
				if (wstrModName.find(wstrModContain) != std::string::npos)
				{
					Global::BaseAddress = hMods[i];
					return true;
				}
			}
		}
	}
	return false;
}

DWORD_PTR Mem::GetBaseAddress() {
	return (DWORD_PTR)Global::BaseAddress;
}

template<typename T> T Mem::RPM(SIZE_T address) {
	//The buffer for data that is going to be read from memory
	T buffer;

	//The actual RPM
	ReadProcessMemory(Global::GameHandle, (LPCVOID)address, &buffer, sizeof(T), NULL);

	//Return our buffer
	return buffer;
}

std::string Mem::RPMString(SIZE_T address) {
	//Make a char array of 20 bytes
	char name[20];

	//The actual RPM
	ReadProcessMemory(Global::GameHandle, (LPCVOID)address, &name, sizeof(name), NULL);

	//Add each char to our string
	//While also checking for a null terminator to end the string
	std::string nameString;
	for (int i = 0; i < sizeof(name); i++) {
		if (name[i] == 0)
			break;
		else
			nameString += name[i];
	}

	return nameString;
}

template<typename T> void Mem::WPM(SIZE_T address, T buffer) {
	//A couple checks to try and avoid crashing
	//These don't actually make sense, feel free to remove redundent ones
	if (address == 0 || (LPVOID)address == nullptr || address == NULL) {
		return;
	}

	WriteProcessMemory(Global::GameHandle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

void Mem::UpdateAddresses() {
	//Game manager pointer from games base address + the GameManager offset
	pGameManager = RPM<DWORD_PTR>(GetBaseAddress() + Offset::GameManager);
	//Entity list pointer from the GameManager + EntityList offset
	pEntityList = RPM<DWORD_PTR>(pGameManager + Offset::EntityList);

	//Renderer pointer from games base address + Renderer offset
	pRender = RPM<DWORD_PTR>(GetBaseAddress() + Offset::Renderer);
	//Game Renderer pointer from Renderer + GameRenderer offset
	pGameRender = RPM<DWORD_PTR>(pRender + Offset::GameRenderer);
	//EngineLink pointer from GameRenderer + EngineLink offset
	pEngineLink = RPM<DWORD_PTR>(pGameRender + Offset::EngineLink);
	//Engine pointer from EngineLink + Engine offset
	pEngine = RPM<DWORD_PTR>(pEngineLink + Offset::Engine);
	//Camera pointer from Engine + Camera offset
	pCamera = RPM<DWORD_PTR>(pEngine + Offset::Camera);
}

DWORD_PTR Mem::GetEntity(int i) {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(pEntityList + (i * Offset::Entity));
	return RPM<DWORD_PTR>(entityBase + Offset::EntityRef);
}

DWORD_PTR Mem::GetLocalEntity() {
	//Loop through the first 12
	for (int i = 0; i < 12; i++) {
		//get current entity
		DWORD_PTR entity = GetEntity(i);
		//get that entity's name
		std::string entityName = GetEntityPlayerName(entity);

		//check it against our local name
		if (strcmp(entityName.c_str(), Global::LocalName.c_str()) == 0) {
			return entity;
		}
	}

	//return the first entity if we didn't find anything
	return GetEntity(0);
}

DWORD Mem::GetEntityHealth(DWORD_PTR entity) {
	//Entity info pointer from the Entity
	DWORD_PTR EntityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	//Main component pointer from the entity info
	DWORD_PTR MainComponent = RPM<DWORD_PTR>(EntityInfo + Offset::MainComponent);
	//Child component pointer form the main component
	DWORD_PTR ChildComponent = RPM<DWORD_PTR>(MainComponent + Offset::ChildComponent);

	//Finally health from the child component
	return RPM<DWORD>(ChildComponent + Offset::Health);
}

Vector3 Mem::GetEntityFeetPosition(DWORD_PTR entity) {
	//We get the feet position straight from the entity
	return RPM<Vector3>(entity + Offset::FeetPosition);
}

Vector3 Mem::GetEntityHeadPosition(DWORD_PTR entity) {
	//We get the head position straight from the entity
	return RPM<Vector3>(entity + Offset::HeadPosition);
}

std::string Mem::GetEntityPlayerName(DWORD_PTR entity) {
	DWORD_PTR playerInfo = RPM<DWORD_PTR>(entity + Offset::PlayerInfo);
	DWORD_PTR playerInfoD = RPM<DWORD_PTR>(playerInfo + Offset::PlayerInfoDeref);

	return RPMString(RPM<DWORD_PTR>(playerInfoD + Offset::PlayerName) + 0x0);
}

BYTE Mem::GetEntityTeamId(DWORD_PTR entity) {
	//Team id comes from player info
	DWORD_PTR playerInfo = RPM<DWORD_PTR>(entity + Offset::PlayerInfo);
	//We have to derefrnce it as 0x0
	DWORD_PTR playerInfoD = RPM<DWORD_PTR>(playerInfo + Offset::PlayerInfoDeref);

	return RPM<BYTE>(playerInfoD + Offset::PlayerTeamId);
}

PlayerInfo Mem::GetAllEntityInfo(DWORD_PTR entity) {
	PlayerInfo p;

	p.Health = GetEntityHealth(entity);
	p.Name = GetEntityPlayerName(entity);
	p.Position = GetEntityFeetPosition(entity);
	p.w2s = WorldToScreen(p.Position);
	p.w2sHead = WorldToScreen(GetEntityHeadPosition(entity));
	p.TeamId = GetEntityTeamId(entity);

	return p;
}

void Mem::ZeroRecoilSpread(DWORD_PTR entity) {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + Offset::MainComponent);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + Offset::weaponComp);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + Offset::WeaponProcessor);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + Offset::Weapon);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + Offset::WeaponInfo);

	float recoil = RPM<float>(weaponInfo + Offset::Recoil);
	float recoil2 = RPM<float>(weaponInfo + Offset::Recoil2);
	Vector4 recoil3 = RPM<Vector4>(weaponInfo + Offset::Recoil3);
	Vector2 adsRecoil = RPM<Vector2>(weaponInfo + Offset::AdsRecoil);

	float spread = RPM<float>(weaponInfo + Offset::Spread);

	//Make sure values aren't already 0 before we set them to 0, avoids wasting some time and writes

	if (recoil != 0) {
		WPM<float>(weaponInfo + Offset::Recoil, 0);
	}

	if (recoil2 != 0) {
		WPM<float>(weaponInfo + Offset::Recoil2, 0);
	}

	if (!(recoil3.x == 0 && recoil3.y == 0 && recoil3.z == 0 && recoil3.w == 0)) {
		WPM<Vector4>(weaponInfo + Offset::Recoil3, Vector4(0, 0, 0, 0));
	}

	if (!(adsRecoil.x == 0 && adsRecoil.y == 0)) {
		WPM<Vector2>(weaponInfo + Offset::AdsRecoil, Vector2(0, 0));
	}

	if (spread != 0) {
		WPM<float>(weaponInfo + Offset::Spread, 0);
	}

}

Vector3 Mem::GetViewTranslation() {
	//View translation comes straight from the camera
	return RPM<Vector3>(pCamera + Offset::ViewTranslastion);
}

Vector3 Mem::GetViewRight() {
	//View right comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewRight);
}

Vector3 Mem::GetViewUp() {
	//View up comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewUp);
}

Vector3 Mem::GetViewForward() {
	//View forward comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewForward);
}

float Mem::GetFOVX() {
	//FOV comes directly from the camera
	return RPM<float>(pCamera + Offset::FOVX);
}

float Mem::GetFOVY() {
	//FOV comes directly from the camera
	return RPM<float>(pCamera + Offset::FOVY);
}

//Shout out to RangeMachine's SDK for this one
Vector3 Mem::WorldToScreen(Vector3 position) {
	Vector3 temp = position - GetViewTranslation();

	float x = temp.Dot(GetViewRight());
	float y = temp.Dot(GetViewUp());
	float z = temp.Dot(GetViewForward() * -1);

	return Vector3((displayWidth / 2) * (1 + x / GetFOVX() / z), (displayHeight / 2) * (1 - y / GetFOVY() / z), z);
}
