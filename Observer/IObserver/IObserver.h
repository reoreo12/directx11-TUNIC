#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

class ISubject;

// Subject로부터 이벤트 알림을 받는 Observer 인터페이스
// 예: 인벤토리, 체크포인트 등에서 발생한 게임 이벤트를 UI, 오브젝트 등이 전달받아 처리
class IObserver abstract
{
public:
	// 이벤트 콜백 순수 가상 함수
	// wsTag: 이벤트를 구분하는 태그
	// pMessage: 추가 정보 전달용 메시지 포인터
	virtual void OnNotify(const wstring& wsTag, void* pMessage) PURE;
};