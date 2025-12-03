#include "Inventory.h"

CInventory::CInventory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CInventory::CInventory(const CInventory& Prototype)
	: CComponent{ Prototype }

{
}

HRESULT CInventory::Add_Item(CItem::ITEM_TYPE eItemType, _uint iAmount)
{
	// 잘못된 타입이나 추가하려는 수량이 0일 경우 예외 처리
	if (eItemType >= CItem::ITEM_END || iAmount == 0)
		return E_FAIL;

	// 이미 같은 타입의 아이템이 인벤토리에 존재한다면, 수량만 증가
	for (auto& item : m_vecItems)
	{
		if (item.pItem->Get_Type() == eItemType)
		{
			item.iAmount += iAmount;

			// 옵저버들에게 아이템 획득 이벤트 공지
			Notify_Observer(L"GainItem");

			return S_OK;
		}
	}

	// 처음 추가하는 타입이라면 새로 생성
	CItem::ITEM_DESC desc{};
	desc.eType = eItemType;
	desc.strName = L"Default";
	desc.iAmount = 1;

	// 아이템 생성
	CItem* pItem = CItem::Create(desc);
	if (!pItem)
		return E_FAIL;

	// 인벤토리의 컨테이너에 삽입
	INVEN_ITEM invenItem{};
	invenItem.pItem = pItem;
	invenItem.iAmount = iAmount;

	m_vecItems.push_back(invenItem);

	// 옵저버들에게 아이템 획득 이벤트 공지
	Notify_Observer(L"GainItem");

	return S_OK;
}

HRESULT CInventory::Remove_Item(CItem::ITEM_TYPE eItemType, _uint iAmount)
{
	for (auto iter = m_vecItems.begin(); iter != m_vecItems.end(); ++iter)
	{
		if (iter->pItem->Get_Type() == eItemType)
		{
			// 보유 수량이 부족하면 실패
			if (iter->iAmount < iAmount)
				return E_FAIL;

			iter->iAmount -= iAmount;

			// 해당 아이템의 수량이 0개가 되면 슬롯 제거 + 인벤토리의 아이템 메모리 해제
			if (iter->iAmount == 0)
			{
				Safe_Release(iter->pItem);
				m_vecItems.erase(iter);
			}

			return S_OK;
		}
	}

	// 해당 타입의 아이템이 없으면 실패 처리
	return E_FAIL;
}

_bool CInventory::Has_Item(CItem::ITEM_TYPE eItemType) const
{
	if (eItemType >= CItem::ITEM_END)
		return false;

	for (const auto& item : m_vecItems)
	{
		if (!item.pItem)
			continue;

		if (item.pItem->Get_Type() == eItemType && item.iAmount > 0)
			return true;
	}

	return false;
}

_uint CInventory::Get_ItemCount(CItem::ITEM_TYPE eItemType) const
{
	if (eItemType >= CItem::ITEM_END)
		return 0;

	for (const auto& item : m_vecItems)
	{
		if (!item.pItem)
			continue;

		if (item.pItem->Get_Type() == eItemType)
			return item.iAmount;
	}

	return 0;
}

HRESULT CInventory::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CInventory::Initialize(void* pArg)
{
	return S_OK;
}

CInventory* CInventory::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInventory* pInstance = new CInventory(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed To Created : CInventory");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CInventory::Clone(void* pArg)
{
	CComponent* pInstance = new CInventory(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed To Cloned : CInventory");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInventory::Free()
{
	__super::Free();

	for (auto& item : m_vecItems)
		Safe_Release(item.pItem);

	m_vecItems.clear();
}