#include "ISubject.h"

HRESULT ISubject::Add_Observer(const wstring& wsTag, IObserver* pObserver)
{
	if (!pObserver)
		return E_FAIL;

	auto& vecObservers = m_mapObserver[wsTag];
	
	// 이미 등록된 Observer인지 중복 검사
	if (find(vecObservers.begin(), vecObservers.end(), pObserver) == vecObservers.end())
	{
		// 등록된 적 없는 Observer라면 등록
		vecObservers.push_back(pObserver);
	}

	return S_OK;
}

void ISubject::Remove_Observer(const wstring& wsTag, IObserver* pObserver)
{
	if (!pObserver)
		return;

	auto iter = m_mapObserver.find(wsTag);
	if (iter == m_mapObserver.end())
		return;

	auto& vecObservers = iter->second;

	// remove-erase로 해당 Observer를 컨테이너에서 제거
	vecObservers.erase(remove(vecObservers.begin(), vecObservers.end(), pObserver), vecObservers.end());
}

HRESULT ISubject::Notify_Observer(const wstring& wsTag, void* pMessage)
{
	auto iter = m_mapObserver.find(wsTag);
	if (iter == m_mapObserver.end())
		return E_FAIL;

	auto& vecObservers = iter->second;

	// 해당 이벤트에 등록된 모든 Observer에게 콜백 호출
	for (IObserver* pObserver : vecObservers)
	{
		if (!pObserver)
			continue;

		pObserver->OnNotify(wsTag, pMessage);
	}

	return S_OK;
}