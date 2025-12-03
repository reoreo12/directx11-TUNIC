#pragma once
#include "Component.h"
#include "Client_Defines.h"
#include "ISubject.h"
#include "Item.h"

BEGIN(Client)

// 아이템을 획득, 저장, 사용할 수 있는 인벤토리 컴포넌트
// 옵저버 패턴을 통해 UI 등에 아이템 획득 이벤트 공지
class CInventory : public CComponent, public ISubject
{
public:
    // 인벤토리 한 칸의 정보를 나타내는 구조체
    typedef struct tagInvenItem
    {
        CItem* pItem = { nullptr }; // 보유 중인 아이템 객체
        _uint  iAmount = { 1 };     // 보유 중인 아이템 수량
    } INVEN_ITEM;

private:
    CInventory(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CInventory(const CInventory& Prototype);
    virtual ~CInventory() = default;

public:
    // 인벤토리 아이템 추가 함수
    HRESULT Add_Item(CItem::ITEM_TYPE eItemType, _uint iAmount = 1);
    // 인벤토리 아이템 제거 함수(아이템을 사용할 때도 호출)
    HRESULT Remove_Item(CItem::ITEM_TYPE eItemType, _uint iAmount = 1);
    // 특정 아이템 보유에 대한 여부 확인 함수
    _bool Has_Item(CItem::ITEM_TYPE eItemType) const;
    // 특정 아이템의 보유 수량 확인 함수
    _uint Get_ItemCount(CItem::ITEM_TYPE eItemType) const;
    // 인벤토리의 아이템 전체를 보여주는 함수
    const vector<INVEN_ITEM>& Get_AllItems() const { return m_vecItems; }

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

private:
    // 인벤토리 슬롯 컨테이너
    vector<INVEN_ITEM> m_vecItems;

public:
    static CInventory* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

END