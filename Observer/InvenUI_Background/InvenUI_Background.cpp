#include "InvenUI_Background.h"
#include "GameInstance.h"
#include "Player.h"
...

void CInvenUI_Background::OnNotify(const wstring& wsTag, void* pMessage)
{
	// 인벤토리가 보내는 "GainItem" 이벤트 알림에만 반응
	if (wsTag == L"GainItem")
	{
		// 획득한 아이템의 아이콘 UI 생성, 배치
		...
	}
}
...