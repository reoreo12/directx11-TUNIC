#include "Item.h"

CItem* CItem::Create(const ITEM_DESC& desc)
{
    CItem* pInstance = new CItem();

    // 전달받은 정보 복사
    pInstance->m_ItemDesc = desc;
    // 아이템 타입에 따라 이름 설정
    switch (pInstance->m_ItemDesc.eType)
    {
    case ITEM_STICK:
        pInstance->m_ItemDesc.strName = L"Stick";
        break;

    case ITEM_SWORD:
        pInstance->m_ItemDesc.strName = L"Sword";
        break;

    case ITEM_SHIELD:
        pInstance->m_ItemDesc.strName = L"Shield";
        break;

    case ITEM_POTION:
        pInstance->m_ItemDesc.strName = L"Potion";
        break;

    default:
        pInstance->m_ItemDesc.strName = L"Default";
        break;
    }

    return pInstance;
}

void CItem::Free()
{
    __super::Free();
}