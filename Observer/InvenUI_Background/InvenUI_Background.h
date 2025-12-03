#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "IObserver.h"
...

BEGIN(Client)

// IObserver를 상속하는 인벤토리 배경 UI
class InvenUI_Background final : public CUIObject, public IObserver
{
...
public:
	// IObserver 인터페이스 구현
	// 인벤토리가 아이템 획득 이벤트 알림을 보내면 호출
	virtual void OnNotify(const wstring& wsTag, void* pMessage) override;
...
};

END