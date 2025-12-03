#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "IObserver.h"

// 특정 이벤트 발생 시, 등록된 Observer에게 알림을 보내는 Subject 인터페이스
// 인벤토리, 체크포인트가 ISubject를 상속해 특정 이벤트가 발생했을 때 등록된 Observer들에게 공지
class ISubject abstract
{
public:
	// 특정 이벤트 태그로 관리할 Observer들을 등록하는 함수
	HRESULT Add_Observer(const wstring& wsTag, IObserver* pObserver);
	// 특정 이벤트 태그에서 Observer를 삭제하는 함수
	void Remove_Observer(const wstring& wsTag, IObserver* pObserver); 
	// 해당 태그에 등록된 모든 Observer에게 이벤트를 공지하는 함수
	// wsTag: 발생한 이벤트의 태그
	// pMessage: 추가 정보 전달용 메시지 포인터(nullptr 가능)
	HRESULT Notify_Observer(const wstring& wsTag, void* pMessage = nullptr);

protected:
	// 이벤트 태그별 등록된 Observer 목록
	map<wstring, vector<IObserver*>> m_mapObserver;
};