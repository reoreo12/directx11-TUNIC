#pragma once
#include "Client_Defines.h"
#include "Base.h"

BEGIN(Client)

// 획득, 저장, 사용이 가능한 아이템
class CItem final : public CBase
{
public:
    // 아이템 종류 열거체
    enum ITEM_TYPE { ITEM_STICK, ITEM_SWORD, ITEM_SHIELD, ITEM_POTION, ITEM_END };

    // 아이템의 정보를 담는 구조체
    typedef struct tagItemDesc
    {
        ITEM_TYPE eType = ITEM_END; // 아이템 타입
        wstring   strName;          // 아이템 이름
        _uint     iAmount = 1;      // 아이템 수량
    } ITEM_DESC;

private:
    CItem() = default;
    virtual ~CItem() = default;

public:
    // 아이템 정보 확인용 함수
    const ITEM_DESC& Get_Desc() const { return m_ItemDesc; }
    ITEM_TYPE        Get_Type() const { return m_ItemDesc.eType; }
    const wstring&   Get_Name() const { return m_ItemDesc.strName; }
    _uint            Get_Amount() const { return m_ItemDesc.iAmount; }

    // 아이템 수량 조절용 함수
    void             Set_Amount(_uint iAmount) { m_ItemDesc.iAmount = iAmount; }
    void             Add_Amount(_uint iAmount) { m_ItemDesc.iAmount += iAmount; }

private:
    // 아이템 객체의 정보 변수
    ITEM_DESC m_ItemDesc;

public:
    static CItem* Create(const ITEM_DESC& desc);
    virtual void  Free() override;
};

END